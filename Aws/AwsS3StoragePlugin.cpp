/**
 * Cloud storage plugins for Orthanc
 * Copyright (C) 2017-2020 Osimis S.A., Belgium
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 **/

#include "AwsS3StoragePlugin.h"

#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include <aws/s3/model/DeleteObjectRequest.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/core/utils/memory/stl/AWSStringStream.h>

#include <boost/lexical_cast.hpp>
#include <iostream>
#include <fstream>

const char* ALLOCATION_TAG = "OrthancS3";
static const char* const PLUGIN_SECTION = "AwsS3Storage";

class AwsS3StoragePlugin : public BaseStoragePlugin
{
public:

  Aws::S3::S3Client       client_;
  std::string             bucketName_;

public:

  AwsS3StoragePlugin(const Aws::S3::S3Client& client, const std::string& bucketName, bool enableLegacyStorageStructure);

  virtual IWriter* GetWriterForObject(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled);
  virtual IReader* GetReaderForObject(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled);
  virtual void DeleteObject(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled);
  virtual const char* GetConfigurationSectionName() {return PLUGIN_SECTION;};
};


class Writer : public IStoragePlugin::IWriter
{
  std::string             path_;
  Aws::S3::S3Client       client_;
  std::string             bucketName_;

public:
  Writer(const Aws::S3::S3Client& client, const std::string& bucketName, const std::string& path)
    : path_(path),
      client_(client),
      bucketName_(bucketName)
  {
  }

  virtual ~Writer()
  {
  }

  virtual void Write(const char* data, size_t size)
  {
    Aws::S3::Model::PutObjectRequest putObjectRequest;

    putObjectRequest.SetBucket(bucketName_.c_str());
    putObjectRequest.SetKey(path_.c_str());

    std::shared_ptr<Aws::StringStream> stream = Aws::MakeShared<Aws::StringStream>(ALLOCATION_TAG, std::ios_base::in | std::ios_base::binary);

    stream->rdbuf()->pubsetbuf(const_cast<char*>(data), size);
    stream->rdbuf()->pubseekpos(size);
    stream->seekg(0);

    putObjectRequest.SetBody(stream);

    auto result = client_.PutObject(putObjectRequest);

    if (!result.IsSuccess())
    {
      throw StoragePluginException(std::string("error while writing file ") + path_ + ": " + result.GetError().GetExceptionName().c_str() + " " + result.GetError().GetMessage().c_str());
    }
  }
};


class Reader : public IStoragePlugin::IReader
{
  std::string             path_;
  Aws::S3::S3Client       client_;
  std::string             bucketName_;

public:
  Reader(const Aws::S3::S3Client& client, const std::string& bucketName, const std::string& path)
    : path_(path),
      client_(client),
      bucketName_(bucketName)
  {
  }

  virtual ~Reader()
  {

  }
  virtual size_t GetSize()
  {
    Aws::S3::Model::ListObjectsRequest listObjectRequest;
    listObjectRequest.SetBucket(bucketName_.c_str());
    listObjectRequest.SetPrefix(path_.c_str());

    auto result = client_.ListObjects(listObjectRequest);

    if (result.IsSuccess())
    {
      Aws::Vector<Aws::S3::Model::Object> objectList =
          result.GetResult().GetContents();

      if (objectList.size() == 1)
      {
        return objectList[0].GetSize();
      }
      else if (objectList.size() > 1)
      {
        throw StoragePluginException(std::string("error while reading file ") + path_ + ": multiple objet with same name !");
      }
      throw StoragePluginException(std::string("error while reading file ") + path_ + ": object not found !");
    }
    else
    {
      throw StoragePluginException(std::string("error while reading file ") + path_ + ": " + result.GetError().GetExceptionName().c_str() + " " + result.GetError().GetMessage().c_str());
    }
  }
  virtual void Read(char* data, size_t size)
  {
    Aws::S3::Model::GetObjectRequest getObjectRequest;
    getObjectRequest.SetBucket(bucketName_.c_str());
    getObjectRequest.SetKey(path_.c_str());

    getObjectRequest.SetResponseStreamFactory(
          [data, size]()
    {
      std::unique_ptr<Aws::StringStream>
          istream(Aws::New<Aws::StringStream>(ALLOCATION_TAG));

      istream->rdbuf()->pubsetbuf(static_cast<char*>(data),
                                  size);

      return istream.release();
    });

    // Get the object
    auto result = client_.GetObject(getObjectRequest);
    if (result.IsSuccess())
    {
    }
    else
    {
      throw StoragePluginException(std::string("error while reading file ") + path_ + ": " + result.GetError().GetExceptionName().c_str() + " " + result.GetError().GetMessage().c_str());
    }
  }

};



const char* AwsS3StoragePluginFactory::GetStoragePluginName()
{
  return "AWS S3 Storage";
}

IStoragePlugin* AwsS3StoragePluginFactory::CreateStoragePlugin(const OrthancPlugins::OrthancConfiguration& orthancConfig)
{
  bool enableLegacyStorageStructure;

  if (!orthancConfig.IsSection(PLUGIN_SECTION))
  {
    OrthancPlugins::LogWarning(std::string(GetStoragePluginName()) + " plugin, section missing.  Plugin is not enabled.");
    return nullptr;
  }

  OrthancPlugins::OrthancConfiguration pluginSection;
  orthancConfig.GetSection(pluginSection, PLUGIN_SECTION);

  if (!BaseStoragePlugin::ReadCommonConfiguration(enableLegacyStorageStructure, pluginSection))
  {
    return nullptr;
  }

  std::string bucketName;
  std::string region;
  std::string accessKey;
  std::string secretKey;

  if (!pluginSection.LookupStringValue(bucketName, "BucketName"))
  {
    OrthancPlugins::LogError("AwsS3Storage/BucketName configuration missing.  Unable to initialize plugin");
    return nullptr;
  }

  if (!pluginSection.LookupStringValue(region, "Region"))
  {
    OrthancPlugins::LogError("AwsS3Storage/Region configuration missing.  Unable to initialize plugin");
    return nullptr;
  }

  if (!pluginSection.LookupStringValue(accessKey, "AccessKey"))
  {
    OrthancPlugins::LogError("AwsS3Storage/AccessKey configuration missing.  Unable to initialize plugin");
    return nullptr;
  }

  if (!pluginSection.LookupStringValue(secretKey, "SecretKey"))
  {
    OrthancPlugins::LogError("AwsS3Storage/SecretKey configuration missing.  Unable to initialize plugin");
    return nullptr;
  }

  std::string endpoint = pluginSection.GetStringValue("Endpoint", "");
  unsigned int connectTimeout = pluginSection.GetUnsignedIntegerValue("ConnectTimeout", 30);
  unsigned int requestTimeout = pluginSection.GetUnsignedIntegerValue("RequestTimeout", 1200);
  bool virtualAddressing = pluginSection.GetBooleanValue("VirtualAddressing", true);
  
  try
  {
    Aws::SDKOptions options;
    Aws::InitAPI(options);

    Aws::Auth::AWSCredentials credentials(accessKey.c_str(), secretKey.c_str());
    Aws::Client::ClientConfiguration configuration;
    configuration.region = region.c_str();
    configuration.scheme = Aws::Http::Scheme::HTTPS;
    configuration.connectTimeoutMs = connectTimeout * 1000;
    configuration.requestTimeoutMs  = requestTimeout * 1000;
    configuration.httpRequestTimeoutMs = requestTimeout * 1000;

    if (!endpoint.empty())
    {
      configuration.endpointOverride = endpoint.c_str();
    }

    Aws::S3::S3Client client(credentials, configuration, Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, virtualAddressing);

    OrthancPlugins::LogInfo("AWS S3 storage initialized");

    return new AwsS3StoragePlugin(client, bucketName, enableLegacyStorageStructure);
  }
  catch (const std::exception& e)
  {
    OrthancPlugins::LogError(std::string("AzureBlobStorage plugin: failed to initialize plugin: ") + e.what());
    return nullptr;
  }

}

AwsS3StoragePlugin::AwsS3StoragePlugin(const Aws::S3::S3Client& client, const std::string& bucketName, bool enableLegacyStorageStructure)
  : BaseStoragePlugin(enableLegacyStorageStructure),
    client_(client),
    bucketName_(bucketName)
{

}

IStoragePlugin::IWriter* AwsS3StoragePlugin::GetWriterForObject(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled)
{
  return new Writer(client_, bucketName_, GetPath(uuid, type, encryptionEnabled));
}

IStoragePlugin::IReader* AwsS3StoragePlugin::GetReaderForObject(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled)
{
  return new Reader(client_, bucketName_, GetPath(uuid, type, encryptionEnabled));
}

void AwsS3StoragePlugin::DeleteObject(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled)
{
  std::string path = GetPath(uuid, type, encryptionEnabled);

  Aws::S3::Model::DeleteObjectRequest deleteObjectRequest;
  deleteObjectRequest.SetBucket(bucketName_.c_str());
  deleteObjectRequest.SetKey(path.c_str());

  auto result = client_.DeleteObject(deleteObjectRequest);

  if (!result.IsSuccess())
  {
    throw StoragePluginException(std::string("error while deleting file ") + path + ": " + result.GetError().GetExceptionName().c_str() + " " + result.GetError().GetMessage().c_str());
  }

}
