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

#if GOOGLE_STORAGE_PLUGIN==1
#include "../Google/GoogleStoragePlugin.h"
#define StoragePluginFactory GoogleStoragePluginFactory
#elif AZURE_STORAGE_PLUGIN==1
#include "../Azure/AzureBlobStoragePlugin.h"
#define StoragePluginFactory AzureBlobStoragePluginFactory
#elif AWS_STORAGE_PLUGIN==1
#include "../Aws/AwsS3StoragePlugin.h"
#define StoragePluginFactory AwsS3StoragePluginFactory
#else
#pragma message(error  "define a plugin")
#endif

#include <string.h>
#include <stdio.h>
#include <string>

#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include "../Common/EncryptionHelpers.h"
#include "../Common/EncryptionConfigurator.h"
#include <SystemToolbox.h>

static std::unique_ptr<IStoragePlugin> plugin;

static std::unique_ptr<EncryptionHelpers> crypto;
static bool cryptoEnabled = false;
static std::string fileSystemRootPath;
static bool migrationFromFileSystemEnabled = false;
static std::string objectsRootPath;

// class to free memory allocated by malloc if an exception occurs
// This is to avoid an issue in which the blob storage read method
// crashed if the buffer was allocated through:
//   auto buffer = std::unique_ptr<void, void(*)(void*)>{malloc(static_cast<uint64_t>(*size)), free};

class ScopedFree
{
  void* buffer_;
public:
  ScopedFree(void* buffer)
  : buffer_(buffer)
  {
  }
  ~ScopedFree()
  {
    free(buffer_);
  }

  void Release() // abandon ownership
  {
    buffer_ = nullptr;
  }
};

static OrthancPluginErrorCode StorageCreate(const char* uuid,
                                            const void* content,
                                            int64_t size,
                                            OrthancPluginContentType type)
{
  try
  {
    std::unique_ptr<IStoragePlugin::IWriter> writer(plugin->GetWriterForObject(uuid, type, cryptoEnabled));

    if (cryptoEnabled)
    {
      std::string encryptedFile;

      try
      {
        crypto->Encrypt(encryptedFile, (const char*)content, size);
      }
      catch (EncryptionException& ex)
      {
        OrthancPlugins::LogError(std::string(StoragePluginFactory::GetStoragePluginName()) + ": error while encrypting object " + std::string(uuid) + ": " + ex.what());
        return OrthancPluginErrorCode_StorageAreaPlugin;
      }

      writer->Write(encryptedFile.data(), encryptedFile.size());
    }
    else
    {
      writer->Write(reinterpret_cast<const char*>(content), size);
    }
  }
  catch (StoragePluginException& ex)
  {
    OrthancPlugins::LogError(std::string(StoragePluginFactory::GetStoragePluginName()) + ": error while creating object " + std::string(uuid) + ": " + ex.what());
    return OrthancPluginErrorCode_StorageAreaPlugin;
  }

  return OrthancPluginErrorCode_Success;
}


static OrthancPluginErrorCode StorageRead(void** content,
                                          int64_t* size,
                                          const char* uuid,
                                          OrthancPluginContentType type)
{
  try
  {
    std::unique_ptr<IStoragePlugin::IReader> reader(plugin->GetReaderForObject(uuid, type, cryptoEnabled));

    size_t fileSize = reader->GetSize();

    if (cryptoEnabled)
    {
      *size = fileSize - crypto->OVERHEAD_SIZE;
    }
    else
    {
      *size = fileSize;
    }

    if (*size <= 0)
    {
      OrthancPlugins::LogError(std::string(StoragePluginFactory::GetStoragePluginName()) + ": error while reading object " + std::string(uuid) + ", size of file is too small: " + boost::lexical_cast<std::string>(fileSize) + " bytes");
      return OrthancPluginErrorCode_StorageAreaPlugin;
    }

    *content = malloc(static_cast<uint64_t>(*size));
    ScopedFree freeContent(*content);

    if (*content == nullptr)
    {
      OrthancPlugins::LogError(std::string(StoragePluginFactory::GetStoragePluginName()) + ": error while reading object " + std::string(uuid) + ", cannot allocate memory of size " + boost::lexical_cast<std::string>(*size) + " bytes");
      return OrthancPluginErrorCode_StorageAreaPlugin;
    }

    if (cryptoEnabled)
    {
      std::vector<char> encrypted(fileSize);
      reader->Read(encrypted.data(), fileSize);

      try
      {
        crypto->Decrypt((char*)(*content), encrypted.data(), fileSize);
      }
      catch (EncryptionException& ex)
      {
        OrthancPlugins::LogError(std::string(StoragePluginFactory::GetStoragePluginName()) + ": error while decrypting object " + std::string(uuid) + ": " + ex.what());
        return OrthancPluginErrorCode_StorageAreaPlugin;
      }
    }
    else
    {
      reader->Read(*(reinterpret_cast<char**>(content)), fileSize);
    }

    // transmit ownership to content
    freeContent.Release();
  }
  catch (StoragePluginException& ex)
  {
    if (migrationFromFileSystemEnabled)
    {
      try
      {
        OrthancPlugins::LogWarning(std::string(StoragePluginFactory::GetStoragePluginName()) + ": error while reading object " + std::string(uuid) + ": " + ex.what() + ", will now try to read it from legacy orthanc storage");
        std::string path = BaseStoragePlugin::GetOrthancFileSystemPath(uuid, fileSystemRootPath);

        std::string stringBuffer;
        Orthanc::SystemToolbox::ReadFile(stringBuffer, path);

        *content = malloc(static_cast<uint64_t>(stringBuffer.size()));
        ScopedFree freeContent(*content);

        if (*content == nullptr)
        {
          OrthancPlugins::LogError(std::string(StoragePluginFactory::GetStoragePluginName()) + ": error while reading object " + std::string(uuid) + ", cannot allocate memory of size " + boost::lexical_cast<std::string>(*size) + " bytes");
          return OrthancPluginErrorCode_StorageAreaPlugin;
        }
        *size = stringBuffer.size();
        memcpy(*content, stringBuffer.data(), stringBuffer.size());

        // transmit ownership to content
        freeContent.Release();

        return OrthancPluginErrorCode_Success;
      }
      catch(Orthanc::OrthancException& e)
      {
        OrthancPlugins::LogError(std::string(StoragePluginFactory::GetStoragePluginName()) + ": error while reading object " + std::string(uuid) + ": " + std::string(e.What()));
        return OrthancPluginErrorCode_StorageAreaPlugin;
      }
    }
    else
    {
      OrthancPlugins::LogError(std::string(StoragePluginFactory::GetStoragePluginName()) + ": error while reading object " + std::string(uuid) + ": " + ex.what());
      return OrthancPluginErrorCode_StorageAreaPlugin;
    }
  }

  return OrthancPluginErrorCode_Success;
}


static OrthancPluginErrorCode StorageRemove(const char* uuid,
                                            OrthancPluginContentType type)
{
  try
  {
    plugin->DeleteObject(uuid, type, cryptoEnabled);
  }
  catch (StoragePluginException& ex)
  {
    if (migrationFromFileSystemEnabled)
    {
      try
      {
        OrthancPlugins::LogWarning(std::string(StoragePluginFactory::GetStoragePluginName()) + ": error while deleting object " + std::string(uuid) + ": " + ex.what() + ", will now try to delete it from legacy orthanc storage");
        namespace fs = boost::filesystem;

        fs::path path = BaseStoragePlugin::GetOrthancFileSystemPath(uuid, fileSystemRootPath);

        try
        {
          fs::remove(path);
        }
        catch (...)
        {
          // Ignore the error
        }

        // Remove the two parent directories, ignoring the error code if
        // these directories are not empty

        try
        {
          boost::system::error_code err;
          fs::remove(path.parent_path(), err);
          fs::remove(path.parent_path().parent_path(), err);
        }
        catch (...)
        {
          // Ignore the error
        }

        return OrthancPluginErrorCode_Success;
      }
      catch(Orthanc::OrthancException& e)
      {
        OrthancPlugins::LogError(std::string(StoragePluginFactory::GetStoragePluginName()) + ": error while deleting object " + std::string(uuid) + ": " + std::string(e.What()));
        return OrthancPluginErrorCode_StorageAreaPlugin;
      }
    }
    else
    {
      OrthancPlugins::LogError(std::string(StoragePluginFactory::GetStoragePluginName()) + ": error while deleting object " + std::string(uuid) + ": " + ex.what());
      return OrthancPluginErrorCode_StorageAreaPlugin;
    }
  }

  return OrthancPluginErrorCode_Success;
}


extern "C"
{
  ORTHANC_PLUGINS_API int32_t OrthancPluginInitialize(OrthancPluginContext* context)
  {
    OrthancPlugins::SetGlobalContext(context);

    Orthanc::Logging::InitializePluginContext(context);

    OrthancPlugins::OrthancConfiguration orthancConfig;

    OrthancPlugins::LogWarning(std::string(StoragePluginFactory::GetStoragePluginName()) + " plugin is initializing");

    /* Check the version of the Orthanc core */
    if (OrthancPluginCheckVersion(context) == 0)
    {
      char info[1024];
      sprintf(info, "Your version of Orthanc (%s) must be above %d.%d.%d to run this plugin",
              context->orthancVersion,
              ORTHANC_PLUGINS_MINIMAL_MAJOR_NUMBER,
              ORTHANC_PLUGINS_MINIMAL_MINOR_NUMBER,
              ORTHANC_PLUGINS_MINIMAL_REVISION_NUMBER);
      OrthancPlugins::LogError(info);
      return -1;
    }

    try
    {
      plugin.reset(StoragePluginFactory::CreateStoragePlugin(orthancConfig));

      if (plugin.get() == nullptr)
      {
        return -1;
      }

      const char* pluginSectionName = plugin->GetConfigurationSectionName();
      static const char* const ENCRYPTION_SECTION = "StorageEncryption";

      if (orthancConfig.IsSection(pluginSectionName))
      {
        OrthancPlugins::OrthancConfiguration pluginSection;
        orthancConfig.GetSection(pluginSection, pluginSectionName);

        migrationFromFileSystemEnabled = pluginSection.GetBooleanValue("MigrationFromFileSystemEnabled", false);

        if (migrationFromFileSystemEnabled)
        {
          fileSystemRootPath = orthancConfig.GetStringValue("StorageDirectory", "OrthancStorageNotDefined");
          OrthancPlugins::LogWarning(std::string(StoragePluginFactory::GetStoragePluginName()) + ": migration from file system enabled, source: " + fileSystemRootPath);
        }

        objectsRootPath = pluginSection.GetStringValue("RootPath", std::string());

        if (objectsRootPath.size() >= 1 && objectsRootPath[0] == '/')
        {
          OrthancPlugins::LogError(std::string(StoragePluginFactory::GetStoragePluginName()) + ": The RootPath shall not start with a '/': " + objectsRootPath);
          return -1;
        }

        plugin->SetRootPath(objectsRootPath);

        if (pluginSection.IsSection(ENCRYPTION_SECTION))
        {
          OrthancPlugins::OrthancConfiguration cryptoSection;
          pluginSection.GetSection(cryptoSection, ENCRYPTION_SECTION);

          crypto.reset(EncryptionConfigurator::CreateEncryptionHelpers(cryptoSection));
          cryptoEnabled = crypto.get() != nullptr;
        }

        if (cryptoEnabled)
        {
          OrthancPlugins::LogWarning(std::string(StoragePluginFactory::GetStoragePluginName()) + ": client-side encryption is enabled");
        }
        else
        {
          OrthancPlugins::LogWarning(std::string(StoragePluginFactory::GetStoragePluginName()) + ": client-side encryption is disabled");
        }
      }

      OrthancPluginRegisterStorageArea(context, StorageCreate, StorageRead, StorageRemove);
    }
    catch (Orthanc::OrthancException& e)
    {
      LOG(ERROR) << "Exception while creating the object storage plugin: " << e.What();
      return -1;
    }

    return 0;
  }


  ORTHANC_PLUGINS_API void OrthancPluginFinalize()
  {
    OrthancPlugins::LogWarning(std::string(StoragePluginFactory::GetStoragePluginName()) + " plugin is finalizing");
  }


  ORTHANC_PLUGINS_API const char* OrthancPluginGetName()
  {
    return StoragePluginFactory::GetStoragePluginName();
  }


  ORTHANC_PLUGINS_API const char* OrthancPluginGetVersion()
  {
    return PLUGIN_VERSION;
  }
}

