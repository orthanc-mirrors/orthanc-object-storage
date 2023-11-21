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

#include "AwsS3StoragePlugin.h"
#include <Logging.h>

#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include <aws/s3/model/DeleteObjectRequest.h>
#include <aws/core/auth/AWSCredentialsProvider.h>
#include <aws/core/utils/HashingUtils.h>
#include <aws/core/utils/logging/DefaultLogSystem.h>
#include <aws/core/utils/logging/DefaultCRTLogSystem.h>
#include <aws/core/utils/logging/AWSLogging.h>
#include <aws/core/utils/memory/stl/AWSStreamFwd.h>
#include <aws/core/utils/memory/stl/AWSStringStream.h>
#include <aws/core/utils/memory/AWSMemory.h>
#include <aws/core/utils/stream/PreallocatedStreamBuf.h>
#include <aws/core/utils/StringUtils.h>
#include <aws/transfer/TransferManager.h>
#include <aws/crt/Api.h>
#include <iostream>
#include <fstream>

#include <boost/lexical_cast.hpp>
#include <boost/interprocess/streams/bufferstream.hpp>
#include <iostream>
#include <fstream>

const char* ALLOCATION_TAG = "OrthancS3";

class AwsS3StoragePlugin : public BaseStorage
{
public:

  std::string             bucketName_;
  bool                    storageContainsUnknownFiles_;
  bool                    useTransferManager_;
  std::shared_ptr<Aws::S3::S3Client>               client_;
  std::shared_ptr<Aws::Utils::Threading::Executor> executor_;
  std::shared_ptr<Aws::Transfer::TransferManager>  transferManager_;

public:

  AwsS3StoragePlugin(const std::string& nameForLogs,  std::shared_ptr<Aws::S3::S3Client> client, const std::string& bucketName, bool enableLegacyStorageStructure, bool storageContainsUnknownFiles, bool useTransferManager, unsigned int transferThreadPoolSize, unsigned int transferBufferSizeMB);

  virtual ~AwsS3StoragePlugin();

  virtual IWriter* GetWriterForObject(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled);
  virtual IReader* GetReaderForObject(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled);
  virtual void DeleteObject(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled);
  virtual bool HasFileExists() {return false;};
};


class DirectWriter : public IStorage::IWriter
{
  std::string             path_;
  std::shared_ptr<Aws::S3::S3Client>       client_;
  std::string             bucketName_;

public:
  DirectWriter(std::shared_ptr<Aws::S3::S3Client> client, const std::string& bucketName, const std::string& path)
    : path_(path),
      client_(client),
      bucketName_(bucketName)
  {
  }

  virtual ~DirectWriter()
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
    putObjectRequest.SetContentMD5(Aws::Utils::HashingUtils::Base64Encode(Aws::Utils::HashingUtils::CalculateMD5(*stream)));

    auto result = client_->PutObject(putObjectRequest);

    if (!result.IsSuccess())
    {
      throw StoragePluginException(std::string("error while writing file ") + path_ + ": response code = " + boost::lexical_cast<std::string>((int)result.GetError().GetResponseCode()) + " " + result.GetError().GetExceptionName().c_str() + " " + result.GetError().GetMessage().c_str());
    }
  }
};


class DirectReader : public IStorage::IReader
{
protected:
  std::shared_ptr<Aws::S3::S3Client>       client_;
  std::string             bucketName_;
  std::list<std::string>  paths_;
  std::string             uuid_;

public:
  DirectReader(std::shared_ptr<Aws::S3::S3Client> client, const std::string& bucketName, const std::list<std::string>& paths, const char* uuid)
    : client_(client),
      bucketName_(bucketName),
      paths_(paths),
      uuid_(uuid)
  {
  }

  virtual ~DirectReader()
  {

  }

  virtual size_t GetSize()
  {
    std::string firstExceptionMessage;

    for (auto& path: paths_)
    {
      try
      {
        return _GetSize(path);
      }
      catch (StoragePluginException& ex)
      {
        if (firstExceptionMessage.empty())
        {
          firstExceptionMessage = ex.what();
        }
        //ignore to retry
      }
    }
    throw StoragePluginException(firstExceptionMessage);
  }

  virtual void ReadWhole(char* data, size_t size)
  {
    _Read(data, size, 0, false);
  }

  virtual void ReadRange(char* data, size_t size, size_t fromOffset)
  {
    _Read(data, size, fromOffset, true);
  }

private:

  size_t _GetSize(const std::string& path)
  {
    Aws::S3::Model::ListObjectsRequest listObjectRequest;
    listObjectRequest.SetBucket(bucketName_.c_str());
    listObjectRequest.SetPrefix(path.c_str());

    auto result = client_->ListObjects(listObjectRequest);

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
        throw StoragePluginException(std::string("error while reading file ") + path + ": multiple objet with same name !");
      }
      throw StoragePluginException(std::string("error while reading file ") + path + ": object not found !");
    }
    else
    {
      throw StoragePluginException(std::string("error while reading file ") + path + ": " + result.GetError().GetExceptionName().c_str() + " " + result.GetError().GetMessage().c_str());
    }
  }


  void _Read(char* data, size_t size, size_t fromOffset, bool useRange)
  {
    std::string firstExceptionMessage;

    for (auto& path: paths_)
    {
      try
      {
        return __Read(path, data, size, fromOffset, useRange);
      }
      catch (StoragePluginException& ex)
      {
        if (firstExceptionMessage.empty())
        {
          firstExceptionMessage = ex.what();
        }
        //ignore to retry
      }
    }
    throw StoragePluginException(firstExceptionMessage);
  }

  void __Read(const std::string& path, char* data, size_t size, size_t fromOffset, bool useRange)
  {
    Aws::S3::Model::GetObjectRequest getObjectRequest;
    getObjectRequest.SetBucket(bucketName_.c_str());
    getObjectRequest.SetKey(path.c_str());

    if (useRange)
    {
      // https://developer.mozilla.org/en-US/docs/Web/HTTP/Range_requests
      std::string range = std::string("bytes=") + boost::lexical_cast<std::string>(fromOffset) + "-" + boost::lexical_cast<std::string>(fromOffset + size -1);
      getObjectRequest.SetRange(range.c_str());
    }

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
    auto result = client_->GetObject(getObjectRequest);
    if (result.IsSuccess())
    {
    }
    else
    {
      throw StoragePluginException(std::string("error while reading file ") + path + ": response code = " + boost::lexical_cast<std::string>((int)result.GetError().GetResponseCode()) + " " + result.GetError().GetExceptionName().c_str() + " " + result.GetError().GetMessage().c_str());
    }
  }

};



class TransferWriter : public IStorage::IWriter
{
  std::string             path_;
  std::shared_ptr<Aws::Transfer::TransferManager>  transferManager_;
  std::string             bucketName_;

public:
  TransferWriter(std::shared_ptr<Aws::Transfer::TransferManager> transferManager, const std::string& bucketName, const std::string& path)
    : path_(path),
      transferManager_(transferManager),
      bucketName_(bucketName)
  {
  }

  virtual ~TransferWriter()
  {
  }

  virtual void Write(const char* data, size_t size)
  {
    boost::interprocess::bufferstream buffer(const_cast<char*>(static_cast<const char*>(data)), static_cast<size_t>(size));
    std::shared_ptr<Aws::IOStream> body = Aws::MakeShared<Aws::IOStream>(ALLOCATION_TAG, buffer.rdbuf());

    std::shared_ptr<Aws::Transfer::TransferHandle> transferHandle = transferManager_->UploadFile(body, bucketName_, path_.c_str(), "application/binary", Aws::Map<Aws::String, Aws::String>());
    transferHandle->WaitUntilFinished();

    if (transferHandle->GetStatus() != Aws::Transfer::TransferStatus::COMPLETED)
    {
      throw StoragePluginException(std::string("error while writing file ") + path_ + ": response code = " + boost::lexical_cast<std::string>(static_cast<int>(transferHandle->GetLastError().GetResponseCode())) + " " + transferHandle->GetLastError().GetMessage());
    }
  }
};


class TransferReader : public DirectReader
{
  std::shared_ptr<Aws::Transfer::TransferManager>  transferManager_;

public:
  TransferReader(std::shared_ptr<Aws::Transfer::TransferManager> transferManager, std::shared_ptr<Aws::S3::S3Client> client, const std::string& bucketName, const std::list<std::string>& paths, const char* uuid)
    : DirectReader(client, bucketName, paths, uuid),
      transferManager_(transferManager)
  {
  }

  virtual ~TransferReader()
  {

  }

  virtual void ReadWhole(char* data, size_t size)
  {
    std::string firstExceptionMessage;

    for (auto& path: paths_)
    {
      try
      {
        // The local variable 'streamBuffer' is captured by reference in a lambda.
        // It must persist until all downloading by the 'transfer_manager' is complete.
        Aws::Utils::Stream::PreallocatedStreamBuf streamBuffer(reinterpret_cast<unsigned char*>(data), size);

        std::shared_ptr<Aws::Transfer::TransferHandle> downloadHandler = transferManager_->DownloadFile(bucketName_, path, [&]() { //Define a lambda expression for the callback method parameter to stream back the data.
                    return Aws::New<Aws::IOStream>(ALLOCATION_TAG, &streamBuffer);
                });
        
        downloadHandler->WaitUntilFinished();

        if (downloadHandler->GetStatus() == Aws::Transfer::TransferStatus::COMPLETED)
        {
          return;
        }
        else if (firstExceptionMessage.empty())
        {
          firstExceptionMessage = downloadHandler->GetLastError().GetMessage();
        }
    // getObjectRequest.SetResponseStreamFactory(
    //       [data, size]()
    // {
    //   std::unique_ptr<Aws::StringStream>
    //       istream(Aws::New<Aws::StringStream>(ALLOCATION_TAG));

    //   istream->rdbuf()->pubsetbuf(static_cast<char*>(data),
    //                               size);

    //   return istream.release();
    // });


      }
      catch (StoragePluginException& ex)
      {
        if (firstExceptionMessage.empty())
        {
          firstExceptionMessage = ex.what();
        }
        //ignore to retry
      }
    }
    throw StoragePluginException(firstExceptionMessage);
  }

};




const char* AwsS3StoragePluginFactory::GetStoragePluginName()
{
  return "AWS S3 Storage";
}

const char* AwsS3StoragePluginFactory::GetStorageDescription()
{
  return "Stores the Orthanc storage area in AWS S3";
}

static std::unique_ptr<Aws::Crt::ApiHandle>  api_;
static std::unique_ptr<Aws::SDKOptions>  sdkOptions_;

#include <stdarg.h>

class AwsOrthancLogger : public Aws::Utils::Logging::LogSystemInterface
{
public:
    virtual ~AwsOrthancLogger() {}

    /**
     * Gets the currently configured log level for this logger.
     */
    virtual Aws::Utils::Logging::LogLevel GetLogLevel() const
    {
      return Aws::Utils::Logging::LogLevel::Trace;
    }
    /**
     * Does a printf style output to the output stream. Don't use this, it's unsafe. See LogStream
     */
    virtual void Log(Aws::Utils::Logging::LogLevel logLevel, const char* tag, const char* formatStr, ...)
    {
      Aws::StringStream ss;

      va_list args;
      va_start(args, formatStr);

      va_list tmp_args; //unfortunately you cannot consume a va_list twice
      va_copy(tmp_args, args); //so we have to copy it
      #ifdef _WIN32
          const int requiredLength = _vscprintf(formatStr, tmp_args) + 1;
      #else
          const int requiredLength = vsnprintf(nullptr, 0, formatStr, tmp_args) + 1;
      #endif
      va_end(tmp_args);

      char outputBuff[requiredLength];
      #ifdef _WIN32
          vsnprintf_s(outputBuff, requiredLength, _TRUNCATE, formatStr, args);
      #else
          vsnprintf(outputBuff, requiredLength, formatStr, args);
      #endif // _WIN32

      if (logLevel == Aws::Utils::Logging::LogLevel::Debug || logLevel == Aws::Utils::Logging::LogLevel::Trace)
      {
        LOG(INFO) << reinterpret_cast<const char*>(&outputBuff[0]);
      }
      else if (logLevel == Aws::Utils::Logging::LogLevel::Warn)
      {
        LOG(WARNING) << reinterpret_cast<const char*>(&outputBuff[0]);
      }
      else
      {
        LOG(ERROR) << reinterpret_cast<const char*>(&outputBuff[0]);
      }

      va_end(args);
    }
    /**
    * Writes the stream to the output stream.
    */
    virtual void LogStream(Aws::Utils::Logging::LogLevel logLevel, const char* tag, const Aws::OStringStream &messageStream)
    {
      if (logLevel == Aws::Utils::Logging::LogLevel::Debug || logLevel == Aws::Utils::Logging::LogLevel::Trace)
      {
        LOG(INFO) << tag << messageStream.str();
      }
      else if (logLevel == Aws::Utils::Logging::LogLevel::Warn)
      {
        LOG(WARNING) << tag << messageStream.str();
      }
      else
      {
        LOG(ERROR) << tag << messageStream.str();
      }

    }
    /**
     * Writes any buffered messages to the underlying device if the logger supports buffering.
     */
    virtual void Flush() {}
};

IStorage* AwsS3StoragePluginFactory::CreateStorage(const std::string& nameForLogs, const OrthancPlugins::OrthancConfiguration& orthancConfig)
{
  if (sdkOptions_.get() != NULL)
  {
    throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls, "Cannot initialize twice");
  }

  bool enableLegacyStorageStructure;
  bool storageContainsUnknownFiles;

  if (!orthancConfig.IsSection(GetConfigurationSectionName()))
  {
    OrthancPlugins::LogWarning(std::string(GetStoragePluginName()) + " plugin, section missing.  Plugin is not enabled.");
    return nullptr;
  }

  OrthancPlugins::OrthancConfiguration pluginSection;
  orthancConfig.GetSection(pluginSection, GetConfigurationSectionName());

  if (!BaseStorage::ReadCommonConfiguration(enableLegacyStorageStructure, storageContainsUnknownFiles, pluginSection))
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

  const std::string endpoint = pluginSection.GetStringValue("Endpoint", "");
  const unsigned int connectTimeout = pluginSection.GetUnsignedIntegerValue("ConnectTimeout", 30);
  const unsigned int requestTimeout = pluginSection.GetUnsignedIntegerValue("RequestTimeout", 1200);
  const bool virtualAddressing = pluginSection.GetBooleanValue("VirtualAddressing", true);
  const bool enableAwsSdkLogs = pluginSection.GetBooleanValue("EnableAwsSdkLogs", false);
  const std::string caFile = orthancConfig.GetStringValue("HttpsCACertificates", "");


  api_.reset(new Aws::Crt::ApiHandle);

  sdkOptions_.reset(new Aws::SDKOptions);
  sdkOptions_->cryptoOptions.initAndCleanupOpenSSL = false;  // Done by the Orthanc framework
  sdkOptions_->httpOptions.initAndCleanupCurl = false;  // Done by the Orthanc framework

  if (enableAwsSdkLogs)
  {
    // Set up logging
    Aws::Utils::Logging::InitializeAWSLogging(Aws::MakeShared<AwsOrthancLogger>(ALLOCATION_TAG));
    // strangely, this seems to disable logging !!!! sdkOptions_->loggingOptions.logLevel = Aws::Utils::Logging::LogLevel::Trace;
  }

  Aws::InitAPI(*sdkOptions_);


  try
  {
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

    if (!caFile.empty())
    {
      configuration.caFile = caFile;
    }
    
    bool useTransferManager = false; // new in v 2.3.0
    unsigned int transferPoolSize = 10;
    unsigned int transferBufferSizeMB = 5;

    pluginSection.LookupBooleanValue(useTransferManager, "UseTransferManager");
    pluginSection.LookupUnsignedIntegerValue(transferPoolSize, "TransferPoolSize");
    pluginSection.LookupUnsignedIntegerValue(transferBufferSizeMB, "TransferBufferSize");


    std::shared_ptr<Aws::S3::S3Client> client;

    if (pluginSection.LookupStringValue(accessKey, "AccessKey") && pluginSection.LookupStringValue(secretKey, "SecretKey"))
    {
      OrthancPlugins::LogInfo("AWS S3 Storage: using credentials from the configuration file");
      Aws::Auth::AWSCredentials credentials(accessKey.c_str(), secretKey.c_str());
      
      client = Aws::MakeShared<Aws::S3::S3Client>(ALLOCATION_TAG, credentials, configuration, Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, virtualAddressing);
    } 
    else
    {
      // when using default credentials, credentials are not checked at startup but only the first time you try to access the bucket !
      OrthancPlugins::LogInfo("AWS S3 Storage: using default credentials provider");
      client = Aws::MakeShared<Aws::S3::S3Client>(ALLOCATION_TAG, configuration, Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, virtualAddressing);
    }  

    OrthancPlugins::LogInfo("AWS S3 storage initialized");

    return new AwsS3StoragePlugin(nameForLogs, client, bucketName, enableLegacyStorageStructure, storageContainsUnknownFiles, useTransferManager, transferPoolSize, transferBufferSizeMB);
  }
  catch (const std::exception& e)
  {
    OrthancPlugins::LogError(std::string("AWS S3 Storage plugin: failed to initialize plugin: ") + e.what());
    return nullptr;
  }

}


AwsS3StoragePlugin::~AwsS3StoragePlugin()
{
  assert(sdkOptions_.get() != NULL);
  Aws::ShutdownAPI(*sdkOptions_);
  api_.reset();
}


AwsS3StoragePlugin::AwsS3StoragePlugin(const std::string& nameForLogs, 
                                       std::shared_ptr<Aws::S3::S3Client> client, 
                                       const std::string& bucketName, 
                                       bool enableLegacyStorageStructure, 
                                       bool storageContainsUnknownFiles, 
                                       bool useTransferManager,
                                       unsigned int transferThreadPoolSize,
                                       unsigned int transferBufferSizeMB)
  : BaseStorage(nameForLogs, enableLegacyStorageStructure),
    bucketName_(bucketName),
    storageContainsUnknownFiles_(storageContainsUnknownFiles),
    useTransferManager_(useTransferManager),
    client_(client)
{
  if (useTransferManager_)
  {
    executor_ = Aws::MakeShared<Aws::Utils::Threading::PooledThreadExecutor>(ALLOCATION_TAG, transferThreadPoolSize);
    Aws::Transfer::TransferManagerConfiguration transferConfig(executor_.get());
    transferConfig.s3Client = client_;
    transferConfig.bufferSize = static_cast<uint64_t>(transferBufferSizeMB) * 1024 * 1024;
    transferConfig.transferBufferMaxHeapSize = static_cast<uint64_t>(transferBufferSizeMB) * 1024 * 1024 * transferThreadPoolSize;

    transferManager_ = Aws::Transfer::TransferManager::Create(transferConfig);
  }
}

IStorage::IWriter* AwsS3StoragePlugin::GetWriterForObject(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled)
{
  if (useTransferManager_)
  {
    return new TransferWriter(transferManager_, bucketName_, GetPath(uuid, type, encryptionEnabled));
  }
  else
  {
    return new DirectWriter(client_, bucketName_, GetPath(uuid, type, encryptionEnabled));
  }
}

IStorage::IReader* AwsS3StoragePlugin::GetReaderForObject(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled)
{
  std::list<std::string> paths;
  paths.push_back(GetPath(uuid, type, encryptionEnabled, false));
  if (storageContainsUnknownFiles_)
  {
    paths.push_back(GetPath(uuid, type, encryptionEnabled, true));
  }

  if (useTransferManager_)
  {
    return new TransferReader(transferManager_, client_, bucketName_, paths, uuid);
  }
  else
  {
    return new DirectReader(client_, bucketName_, paths, uuid);
  }
}

void AwsS3StoragePlugin::DeleteObject(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled)
{
  std::string firstExceptionMessage;

  std::list<std::string> paths;
  paths.push_back(GetPath(uuid, type, encryptionEnabled, false));
  if (storageContainsUnknownFiles_)
  {
    paths.push_back(GetPath(uuid, type, encryptionEnabled, true));
  }

  // DeleteObject succeeds even if the file does not exist -> we need to try to delete every path
  for (auto& path: paths)
  {
    Aws::S3::Model::DeleteObjectRequest deleteObjectRequest;
    deleteObjectRequest.SetBucket(bucketName_.c_str());
    deleteObjectRequest.SetKey(path.c_str());

    auto result = client_->DeleteObject(deleteObjectRequest);

    if (!result.IsSuccess() && firstExceptionMessage.empty())  
    {
      firstExceptionMessage = std::string("error while deleting file ") + path + ": response code = " + boost::lexical_cast<std::string>((int)result.GetError().GetResponseCode()) + " " + result.GetError().GetExceptionName().c_str() + " " + result.GetError().GetMessage().c_str();
    }
  }

  if (!firstExceptionMessage.empty())
  {
    throw StoragePluginException(firstExceptionMessage);
  }
}
