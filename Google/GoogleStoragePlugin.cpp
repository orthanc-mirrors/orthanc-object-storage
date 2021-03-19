/**
 * Cloud storage plugins for Orthanc
 * Copyright (C) 2020-2021 Osimis S.A., Belgium
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

#include "GoogleStoragePlugin.h"

#include "google/cloud/storage/client.h"

// Create aliases to make the code easier to read.
namespace gcs = google::cloud::storage;
static const char* const PLUGIN_SECTION = "GoogleCloudStorage";

const char* GoogleStoragePlugin::GetConfigurationSectionName() 
{
  return PLUGIN_SECTION;
}

class Writer : public IStoragePlugin::IWriter
{
  std::string   path_;
  gcs::Client   client_;
  std::string   bucketName_;
  gcs::ObjectWriteStream stream_;
public:
  Writer(const std::string& bucketName, const std::string& path, gcs::Client& client)
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
    stream_ = client_.WriteObject(bucketName_, path_);

    if (stream_)
    {
      stream_.write(data, size);
      stream_.Close();

      if (!stream_.metadata())
      {
        throw StoragePluginException("GoogleCloudStorage: error while writing file " + std::string(path_) + ": " + stream_.metadata().status().message());
      }
    }
    else
    {
      throw StoragePluginException("GoogleCloudStorage: error while opening/writing file " + std::string(path_) + ": " + stream_.metadata().status().message());
    }
  }
};


class Reader : public IStoragePlugin::IReader
{
  std::string   path_;
  gcs::Client   client_;
  std::string   bucketName_;

public:
  Reader(const std::string& bucketName, const std::string& path, gcs::Client& client)
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
    auto objectMetadata = client_.GetObjectMetadata(bucketName_, path_);

    if (objectMetadata)
    {
      std::uint64_t fileSize = static_cast<int64_t>(objectMetadata->size());

      return fileSize;
    }
    else
    {
        throw StoragePluginException("error while getting the size of " + std::string(path_) + ": " + objectMetadata.status().message());
    }
  }

  virtual void ReadWhole(char* data, size_t size)
  {
    auto reader = client_.ReadObject(bucketName_, path_);

    if (!reader)
    {
      throw StoragePluginException("error while opening/reading file " + std::string(path_) + ": " + reader.status().message());
    }

    reader.read(data, size);

    if (!reader)
    {
      throw StoragePluginException("error while reading file " + std::string(path_) + ": " + reader.status().message());
    }
  }

  virtual void ReadRange(char* data, size_t size, size_t fromOffset)
  {
    auto reader = client_.ReadObject(bucketName_, path_, gcs::ReadRange(fromOffset, fromOffset + size));

    if (!reader)
    {
      throw StoragePluginException("error while opening/reading file " + std::string(path_) + ": " + reader.status().message());
    }

    reader.read(data, size);

    if (!reader)
    {
      throw StoragePluginException("error while reading file " + std::string(path_) + ": " + reader.status().message());
    }
  }


};



const char* GoogleStoragePluginFactory::GetStoragePluginName()
{
  return "Google Cloud Storage";
}

IStoragePlugin* GoogleStoragePluginFactory::CreateStoragePlugin(const OrthancPlugins::OrthancConfiguration& orthancConfig)
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

  std::string pathToGoogleCredentials;

  if (!pluginSection.LookupStringValue(pathToGoogleCredentials, "ServiceAccountFile"))
  {
    OrthancPlugins::LogError("GoogleCloudStorage/ServiceAccountFile configuration missing.  Unable to initialize plugin");
    return nullptr;
  }

  std::string googleBucketName;
  if (!pluginSection.LookupStringValue(googleBucketName, "BucketName"))
  {
    OrthancPlugins::LogError("GoogleCloudStorage/BucketName configuration missing.  Unable to initialize plugin");
    return nullptr;
  }

  // Use service account credentials from a JSON keyfile:
  auto creds = gcs::oauth2::CreateServiceAccountCredentialsFromJsonFilePath(pathToGoogleCredentials);
  if (!creds)
  {
    OrthancPlugins::LogError("GoogleCloudStorage plugin: unable to validate credentials.  Check the ServiceAccountFile: " + creds.status().message());
    return nullptr;
  }

  // Create a client to communicate with Google Cloud Storage.
  google::cloud::StatusOr<gcs::Client> mainClient = gcs::Client(gcs::ClientOptions(*creds));

  if (!mainClient)
  {
    OrthancPlugins::LogError("GoogleCloudStorage plugin: unable to create client: " + mainClient.status().message());
    return nullptr;
  }

  return new GoogleStoragePlugin(googleBucketName, mainClient.value(), enableLegacyStorageStructure);
}

GoogleStoragePlugin::GoogleStoragePlugin(const std::string &bucketName, google::cloud::storage::Client& mainClient, bool enableLegacyStorageStructure)
  : BaseStoragePlugin(enableLegacyStorageStructure),
    bucketName_(bucketName),
    mainClient_(mainClient)
{

}

IStoragePlugin::IWriter* GoogleStoragePlugin::GetWriterForObject(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled)
{
  return new Writer(bucketName_, GetPath(uuid, type, encryptionEnabled), mainClient_);
}

IStoragePlugin::IReader* GoogleStoragePlugin::GetReaderForObject(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled)
{
  return new Reader(bucketName_, GetPath(uuid, type, encryptionEnabled), mainClient_);
}

void GoogleStoragePlugin::DeleteObject(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled)
{
  gcs::Client client(mainClient_);

  std::string path = GetPath(uuid, type, encryptionEnabled);

  auto deletionStatus = client.DeleteObject(bucketName_, path);

  if (!deletionStatus.ok())
  {
    throw StoragePluginException("GoogleCloudStorage: error while deleting file " + std::string(path) + ": " + deletionStatus.message());
  }

}
