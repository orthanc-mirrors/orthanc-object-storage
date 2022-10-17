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


#include "AzureBlobStoragePlugin.h"

#include <was/storage_account.h>
#include <was/blob.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>
#include "cpprest/rawptrstream.h"
#include "cpprest/details/basic_types.h"


// Create aliases to make the code easier to read.
namespace as = azure::storage;

class AzureBlobStoragePlugin : public BaseStorage
{
public:

  as::cloud_blob_client       blobClient_;
  as::cloud_blob_container    blobContainer_;
  bool                        storageContainsUnknownFiles_;

public:

  AzureBlobStoragePlugin(const std::string& nameForLogs,  
                         const as::cloud_blob_client& blobClient, 
                         const as::cloud_blob_container& blobContainer, 
                         bool enableLegacyStorageStructure,
                         bool storageContainsUnknownFiles
                         );

  virtual IWriter* GetWriterForObject(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled);
  virtual IReader* GetReaderForObject(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled);
  virtual void DeleteObject(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled);
};


class Writer : public IStorage::IWriter
{
  std::string   path_;
  as::cloud_blob_client   client_;
  as::cloud_blob_container container_;

public:
  Writer(const as::cloud_blob_container& container, const std::string& path, const as::cloud_blob_client& client)
    : path_(path),
      client_(client),
      container_(container)
  {
  }

  virtual ~Writer()
  {
  }

  virtual void Write(const char* data, size_t size)
  {
    try
    {
      concurrency::streams::istream inputStream = concurrency::streams::rawptr_stream<uint8_t>::open_istream(reinterpret_cast<const uint8_t*>(data), size);
      azure::storage::cloud_block_blob blob = container_.get_block_blob_reference(utility::conversions::to_string_t(path_));
      blob.upload_from_stream(inputStream);
      inputStream.close().wait();
    }
    catch (std::exception& ex)
    {
      throw StoragePluginException("AzureBlobStorage: error writing file " + std::string(path_) + ": " + ex.what());
    }
  }
};


class Reader : public IStorage::IReader
{
  std::string path_;
  as::cloud_blob_client   client_;
  as::cloud_blob_container container_;
  as::cloud_block_blob block_;

public:
  Reader(const as::cloud_blob_container& container, const std::list<std::string>& paths, const as::cloud_blob_client& client)
    : client_(client),
      container_(container)
  {
    std::string firstExceptionMessage;

    for (auto& path: paths)
    {
      try
      {
        block_ = container_.get_block_blob_reference(utility::conversions::to_string_t(path));
        block_.download_attributes(); // to retrieve the properties
        path_ = path;
        return;
      }
      catch (std::exception& ex)
      {
        if (firstExceptionMessage.empty())
        {
          firstExceptionMessage = "AzureBlobStorage: error opening file for reading " + std::string(path) + ": " + ex.what();
        }
        //ignore to retry
      }
    }
    throw StoragePluginException(firstExceptionMessage);
  }

  virtual ~Reader()
  {
  }

  virtual size_t GetSize()
  {
    try
    {
      return block_.properties().size();
    }
    catch (std::exception& ex)
    {
      throw StoragePluginException("AzureBlobStorage: error while reading file " + std::string(path_) + ": " + ex.what());
    }
  }

  virtual void ReadWhole(char* data, size_t size)
  {
    try
    {
      concurrency::streams::ostream outputStream = concurrency::streams::rawptr_stream<uint8_t>::open_ostream(reinterpret_cast<uint8_t*>(data), size);

      block_.download_to_stream(outputStream);
    }
    catch (std::exception& ex)
    {
      throw StoragePluginException("AzureBlobStorage: error while reading file " + std::string(path_) + ": " + ex.what());
    }
  }

  virtual void ReadRange(char* data, size_t size, size_t fromOffset)
  {
    try
    {
      concurrency::streams::ostream outputStream = concurrency::streams::rawptr_stream<uint8_t>::open_ostream(reinterpret_cast<uint8_t*>(data), size);

      block_.download_range_to_stream(outputStream, fromOffset, size);
    }
    catch (std::exception& ex)
    {
      throw StoragePluginException("AzureBlobStorage: error while reading partial file " + std::string(path_) + ": " + ex.what());
    }
  }
};



const char* AzureBlobStoragePluginFactory::GetStoragePluginName()
{
  return "Azure Blob Storage";
}

bool IsSasTokenAccountLevel(utility::string_t sasToken)
{
  // Use cpprestsdk's utility::string_t here since the expected argument is the return value of
  // as::storage_credentials::sas_token(), which is type utility::string_t
  size_t newIdx = 0;
  size_t prevIdx = 0;
  while ((newIdx = sasToken.find('&', prevIdx)) != utility::string_t::npos)
  {
    utility::string_t kvpair = sasToken.substr(prevIdx, newIdx - prevIdx);
    prevIdx = newIdx + 1; // start next time from char after '&'

    size_t equalsIdx = kvpair.find('=');
    utility::string_t key = kvpair.substr(0, equalsIdx);
    if (key == "srt") // only account SAS has this parameter
      return true;
  }

  return false;
}

IStorage* AzureBlobStoragePluginFactory::CreateStorage(const std::string& nameForLogs, const OrthancPlugins::OrthancConfiguration& orthancConfig)
{
  std::string connectionString;
  std::string containerName;
  bool enableLegacyStorageStructure;
  bool storageContainsUnknownFiles;
  bool createContainerIfNotExists;

  if (orthancConfig.IsSection(GetConfigurationSectionName()))
  {
    OrthancPlugins::OrthancConfiguration pluginSection;
    orthancConfig.GetSection(pluginSection, GetConfigurationSectionName());

    if (!BaseStorage::ReadCommonConfiguration(enableLegacyStorageStructure, storageContainsUnknownFiles, pluginSection))
    {
      return nullptr;
    }

    if (!pluginSection.LookupStringValue(connectionString, "ConnectionString"))
    {
      OrthancPlugins::LogError("AzureBlobStorage/ConnectionString configuration missing.  Unable to initialize plugin");
      return nullptr;
    }

    if (!pluginSection.LookupStringValue(containerName, "ContainerName"))
    {
      OrthancPlugins::LogError("AzureBlobStorage/ContainerName configuration missing.  Unable to initialize plugin");
      return nullptr;
    }

    boost::trim(connectionString); // without that, if there's an EOL in the string, it fails with "provided uri is invalid"
    boost::trim(containerName);

    createContainerIfNotExists = pluginSection.GetBooleanValue("CreateContainerIfNotExists", true);
  }
  else if (orthancConfig.IsSection("BlobStorage")) // backward compatibility with the old plugin:
  {
    OrthancPlugins::LogWarning("AzureBlobStorage: you're using an old configuration format for the plugin.");

    OrthancPlugins::OrthancConfiguration pluginSection;
    orthancConfig.GetSection(pluginSection, "BlobStorage");

    std::string accountName;
    std::string accountKey;

    if (!pluginSection.LookupStringValue(containerName, "ContainerName"))
    {
      OrthancPlugins::LogError("BlobStorage/AccountName configuration missing.  Unable to initialize plugin");
      return nullptr;
    }

    if (!pluginSection.LookupStringValue(accountName, "AccountName"))
    {
      OrthancPlugins::LogError("BlobStorage/AccountName configuration missing.  Unable to initialize plugin");
      return nullptr;
    }

    if (!pluginSection.LookupStringValue(accountKey, "AccountKey"))
    {
      OrthancPlugins::LogError("BlobStorage/ContainerName configuration missing.  Unable to initialize plugin");
      return nullptr;
    }

    std::ostringstream connectionStringBuilder;
    connectionStringBuilder << "DefaultEndpointsProtocol=https;AccountName=" << accountName << ";AccountKey=" << accountKey;
    connectionString = connectionStringBuilder.str();

    createContainerIfNotExists = pluginSection.GetBooleanValue("CreateContainerIfNotExists", true);
  }
  else
  {
    OrthancPlugins::LogWarning(std::string(GetStoragePluginName()) + " plugin, section missing.  Plugin is not enabled.");
    return nullptr;
  }

  try
  {
    OrthancPlugins::LogInfo("Connecting to Azure storage ...");

    as::cloud_storage_account storageAccount = as::cloud_storage_account::parse(utility::conversions::to_string_t(connectionString));
    OrthancPlugins::LogInfo("Storage account created");

    as::cloud_blob_client blobClient = storageAccount.create_cloud_blob_client();
    OrthancPlugins::LogInfo("Blob client created");

    as::cloud_blob_container blobContainer = blobClient.get_container_reference(utility::conversions::to_string_t(containerName));
    OrthancPlugins::LogInfo("Accessing blob container");

    // blobContainer.create_if_not_exists() throws an error if a service SAS (for a blob container)
    // was used in the connectionString.
    // Only allow the use of this function when using storage account-level credentials, whether
    // through accountName/accountKey combo or account SAS.
    if ((storageAccount.credentials().is_account_key() ||
         (storageAccount.credentials().is_sas() && IsSasTokenAccountLevel(storageAccount.credentials().sas_token())))
        && createContainerIfNotExists)
    {
      // Return value is true if the container did not exist and was successfully created.
      bool containerCreated = blobContainer.create_if_not_exists();

      if (containerCreated)
      {
        OrthancPlugins::LogWarning("Blob Storage Area container has been created.  **** check in the Azure console that your container is private ****");
      }
    }

    OrthancPlugins::LogInfo("Blob storage initialized");

    return new AzureBlobStoragePlugin(nameForLogs, blobClient, blobContainer, enableLegacyStorageStructure, storageContainsUnknownFiles);
  }
  catch (const std::exception& e)
  {
    OrthancPlugins::LogError(std::string("AzureBlobStorage plugin: failed to initialize plugin: ") + e.what());
    return nullptr;
  }

}

AzureBlobStoragePlugin::AzureBlobStoragePlugin(const std::string& nameForLogs, const as::cloud_blob_client& blobClient, const as::cloud_blob_container& blobContainer, bool enableLegacyStorageStructure, bool storageContainsUnknownFiles)
  : BaseStorage(nameForLogs, enableLegacyStorageStructure),
    blobClient_(blobClient),
    blobContainer_(blobContainer),
    storageContainsUnknownFiles_(storageContainsUnknownFiles)
{

}

IStorage::IWriter* AzureBlobStoragePlugin::GetWriterForObject(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled)
{
  return new Writer(blobContainer_, GetPath(uuid, type, encryptionEnabled), blobClient_);
}

IStorage::IReader* AzureBlobStoragePlugin::GetReaderForObject(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled)
{
  std::list<std::string> paths;
  paths.push_back(GetPath(uuid, type, encryptionEnabled, false));
  if (storageContainsUnknownFiles_)
  {
    paths.push_back(GetPath(uuid, type, encryptionEnabled, true));
  }

  return new Reader(blobContainer_, paths, blobClient_);
}

void AzureBlobStoragePlugin::DeleteObject(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled)
{
  std::string path = GetPath(uuid, type, encryptionEnabled);

  try
  {
    as::cloud_block_blob blockBlob = blobContainer_.get_block_blob_reference(utility::conversions::to_string_t(path));

    blockBlob.delete_blob();
  }
  catch (std::exception& ex)
  {
    throw StoragePluginException("AzureBlobStorage: error while deleting file " + std::string(path) + ": " + ex.what());
  }
}
