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

#include "AzureBlobStoragePlugin.h"

#include <was/storage_account.h>
#include <was/blob.h>
#include <boost/lexical_cast.hpp>
#include "cpprest/rawptrstream.h"


// Create aliases to make the code easier to read.
namespace as = azure::storage;


class AzureBlobStoragePlugin : public IStoragePlugin
{
public:

  as::cloud_blob_client       blobClient_;
  as::cloud_blob_container    blobContainer_;
public:

//  AzureBlobStoragePlugin(const std::string& connectionString,
//                         const std::string& containerName
//                        );
  AzureBlobStoragePlugin(const as::cloud_blob_client& blobClient, const as::cloud_blob_container& blobContainer);

  virtual IWriter* GetWriterForObject(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled);
  virtual IReader* GetReaderForObject(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled);
  virtual void DeleteObject(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled);
private:
  virtual std::string GetPath(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled);
};


class Writer : public IStoragePlugin::IWriter
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
      azure::storage::cloud_block_blob blob = container_.get_block_blob_reference(path_);
      blob.upload_from_stream(inputStream);
      inputStream.close().wait();
    }
    catch (std::exception& ex)
    {
      throw StoragePluginException("AzureBlobStorage: error writing file " + std::string(path_) + ": " + ex.what());
    }
  }
};


class Reader : public IStoragePlugin::IReader
{
  std::string   path_;
  as::cloud_blob_client   client_;
  as::cloud_blob_container container_;
  as::cloud_block_blob block_;

public:
  Reader(const as::cloud_blob_container& container, const std::string& path, const as::cloud_blob_client& client)
    : path_(path),
      client_(client),
      container_(container)
  {
    try
    {
      block_ = container_.get_block_blob_reference(path_);
      block_.download_attributes(); // to retrieve the properties
    }
    catch (std::exception& ex)
    {
      throw StoragePluginException("AzureBlobStorage: error opening file for reading " + std::string(path_) + ": " + ex.what());
    }
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

  virtual void Read(char* data, size_t size)
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

};



const char* AzureBlobStoragePluginFactory::GetStoragePluginName()
{
  return "Azure Blob Storage";
}

IStoragePlugin* AzureBlobStoragePluginFactory::CreateStoragePlugin(const OrthancPlugins::OrthancConfiguration& orthancConfig)
{
  std::string connectionString;
  std::string containerName;

  static const char* const PLUGIN_SECTION = "AzureBlobStorage";
  if (orthancConfig.IsSection(PLUGIN_SECTION))
  {
    OrthancPlugins::OrthancConfiguration pluginSection;
    orthancConfig.GetSection(pluginSection, PLUGIN_SECTION);

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
  }
  else
  {
    OrthancPlugins::LogWarning(std::string(GetStoragePluginName()) + " plugin, section missing.  Plugin is not enabled.");
    return nullptr;
  }

  try
  {
    OrthancPlugins::LogInfo("Connecting to Azure storage ...");

    as::cloud_storage_account storageAccount = as::cloud_storage_account::parse(connectionString);
    OrthancPlugins::LogInfo("Storage account created");

    as::cloud_blob_client blobClient = storageAccount.create_cloud_blob_client();
    OrthancPlugins::LogInfo("Blob client created");

    as::cloud_blob_container blobContainer = blobClient.get_container_reference(containerName);
    OrthancPlugins::LogInfo("Accessing blob container");

    // Return value is true if the container did not exist and was successfully created.
    bool containerCreated = blobContainer.create_if_not_exists();

    if (containerCreated)
    {
      OrthancPlugins::LogWarning("Blob Storage Area container has been created.  **** check in the Azure console that your container is private ****");
    }

    OrthancPlugins::LogInfo("Blob storage initialized");

    return new AzureBlobStoragePlugin(blobClient, blobContainer);
  }
  catch (const std::exception& e)
  {
    OrthancPlugins::LogError(std::string("AzureBlobStorage plugin: failed to initialize plugin: ") + e.what());
    return nullptr;
  }

}

AzureBlobStoragePlugin::AzureBlobStoragePlugin(const as::cloud_blob_client& blobClient, const as::cloud_blob_container& blobContainer) //const std::string &containerName) //, google::cloud::storage::Client& mainClient)
  : blobClient_(blobClient),
    blobContainer_(blobContainer)
{

}

IStoragePlugin::IWriter* AzureBlobStoragePlugin::GetWriterForObject(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled)
{
  return new Writer(blobContainer_, GetPath(uuid, type, encryptionEnabled), blobClient_);
}

IStoragePlugin::IReader* AzureBlobStoragePlugin::GetReaderForObject(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled)
{
  return new Reader(blobContainer_, GetPath(uuid, type, encryptionEnabled), blobClient_);
}

void AzureBlobStoragePlugin::DeleteObject(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled)
{
  std::string path = GetPath(uuid, type, encryptionEnabled);

  try
  {
    as::cloud_block_blob blockBlob = blobContainer_.get_block_blob_reference(path);

    blockBlob.delete_blob();
  }
  catch (std::exception& ex)
  {
    throw StoragePluginException("AzureBlobStorage: error while deleting file " + std::string(path) + ": " + ex.what());
  }
}

std::string AzureBlobStoragePlugin::GetPath(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled)
{
  std::string path = std::string(uuid);

  if (type == OrthancPluginContentType_Dicom)
  {
    path += ".dcm";
  }
  else if (type == OrthancPluginContentType_DicomAsJson)
  {
    path += ".json";
  }
  else
  {
    path += ".unk";
  }

  if (encryptionEnabled)
  {
    path += ".enc";
  }
  return path;
}
