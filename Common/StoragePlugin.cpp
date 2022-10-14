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


#if GOOGLE_STORAGE_PLUGIN==1
#  include "../Google/GoogleStoragePlugin.h"
#  define StoragePluginFactory GoogleStoragePluginFactory
#elif AZURE_STORAGE_PLUGIN==1
#  include "../Azure/AzureBlobStoragePlugin.h"
#  define StoragePluginFactory AzureBlobStoragePluginFactory
#elif AWS_STORAGE_PLUGIN==1
#  include "../Aws/AwsS3StoragePlugin.h"
#  define StoragePluginFactory AwsS3StoragePluginFactory
#else
#  pragma message(error  "define a plugin")
#endif

#include <string.h>
#include <stdio.h>
#include <string>

#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>

#include "../Common/EncryptionHelpers.h"
#include "../Common/EncryptionConfigurator.h"
#include "../Common/FileSystemStoragePlugin.h"

#include <Logging.h>
#include <SystemToolbox.h>

static std::unique_ptr<IStoragePlugin> primaryPlugin;
static std::unique_ptr<IStoragePlugin> secondaryPlugin;

static std::unique_ptr<EncryptionHelpers> crypto;
static bool cryptoEnabled = false;
static std::string fileSystemRootPath;
static std::string objectsRootPath;
static std::string hybridModeNameForLogs = "";

typedef enum 
{
  HybridMode_WriteToFileSystem,       // write to disk, try to read first from disk and then, from object-storage
  HybridMode_WriteToObjectStorage,    // write to object storage, try to read first from object storage and then, from disk
  HybridMode_Disabled                 // read and write only from/to object-storage
} HybridMode;  

static HybridMode hybridMode = HybridMode_Disabled;

static bool IsReadFromDisk()
{
  return hybridMode != HybridMode_Disabled;
}

static bool IsHybridModeEnabled()
{
  return hybridMode != HybridMode_Disabled;
}

typedef void LogErrorFunction(const std::string& message);



static OrthancPluginErrorCode StorageCreate(const char* uuid,
                                            const void* content,
                                            int64_t size,
                                            OrthancPluginContentType type)
{
  try
  {
    OrthancPlugins::LogInfo(primaryPlugin->GetNameForLogs() + ": creating attachment " + std::string(uuid) + " of type " + boost::lexical_cast<std::string>(type));
    std::unique_ptr<IStoragePlugin::IWriter> writer(primaryPlugin->GetWriterForObject(uuid, type, cryptoEnabled));

    if (cryptoEnabled)
    {
      std::string encryptedFile;

      try
      {
        crypto->Encrypt(encryptedFile, (const char*)content, size);
      }
      catch (EncryptionException& ex)
      {
        OrthancPlugins::LogError(primaryPlugin->GetNameForLogs() + ": error while encrypting object " + std::string(uuid) + ": " + ex.what());
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
    OrthancPlugins::LogError(primaryPlugin->GetNameForLogs() + ": error while creating object " + std::string(uuid) + ": " + ex.what());
    return OrthancPluginErrorCode_StorageAreaPlugin;
  }

  return OrthancPluginErrorCode_Success;
}


static OrthancPluginErrorCode StorageReadRange(IStoragePlugin* plugin,
                                               LogErrorFunction logErrorFunction,
                                               OrthancPluginMemoryBuffer64* target, // Memory buffer where to store the content of the range.  The memory buffer is allocated and freed by Orthanc. The length of the range of interest corresponds to the size of this buffer.
                                               const char* uuid,
                                               OrthancPluginContentType type,
                                               uint64_t rangeStart)
{
  assert(!cryptoEnabled);

  try
  {
    OrthancPlugins::LogInfo(plugin->GetNameForLogs() + ": reading range of attachment " + std::string(uuid) + " of type " + boost::lexical_cast<std::string>(type));
    
    std::unique_ptr<IStoragePlugin::IReader> reader(plugin->GetReaderForObject(uuid, type, cryptoEnabled));
    reader->ReadRange(reinterpret_cast<char*>(target->data), target->size, rangeStart);
    
    return OrthancPluginErrorCode_Success;
  }
  catch (StoragePluginException& ex)
  {
    logErrorFunction(std::string(StoragePluginFactory::GetStoragePluginName()) + ": error while reading object " + std::string(uuid) + ": " + std::string(ex.what()));
    return OrthancPluginErrorCode_StorageAreaPlugin;
  }
}

static OrthancPluginErrorCode StorageReadRange(OrthancPluginMemoryBuffer64* target, // Memory buffer where to store the content of the range.  The memory buffer is allocated and freed by Orthanc. The length of the range of interest corresponds to the size of this buffer.
                                               const char* uuid,
                                               OrthancPluginContentType type,
                                               uint64_t rangeStart)
{
  OrthancPluginErrorCode res = StorageReadRange(primaryPlugin.get(),
                                                (IsHybridModeEnabled() ? OrthancPlugins::LogWarning : OrthancPlugins::LogError), // log errors as warning on first try
                                                target,
                                                uuid,
                                                type,
                                                rangeStart);

  if (res != OrthancPluginErrorCode_Success && IsHybridModeEnabled())
  {
    res = StorageReadRange(secondaryPlugin.get(),
                           OrthancPlugins::LogError, // log errors as errors on second try
                           target,
                           uuid,
                           type,
                           rangeStart);
  }
  return res;
}



static OrthancPluginErrorCode StorageReadWhole(IStoragePlugin* plugin,
                                               LogErrorFunction logErrorFunction,
                                               OrthancPluginMemoryBuffer64* target, // Memory buffer where to store the content of the file. It must be allocated by the plugin using OrthancPluginCreateMemoryBuffer64(). The core of Orthanc will free it.
                                               const char* uuid,
                                               OrthancPluginContentType type)
{
  try
  {
    OrthancPlugins::LogInfo(plugin->GetNameForLogs() + ": reading whole attachment " + std::string(uuid) + " of type " + boost::lexical_cast<std::string>(type));
    std::unique_ptr<IStoragePlugin::IReader> reader(plugin->GetReaderForObject(uuid, type, cryptoEnabled));

    size_t fileSize = reader->GetSize();
    size_t size;

    if (cryptoEnabled)
    {
      size = fileSize - crypto->OVERHEAD_SIZE;
    }
    else
    {
      size = fileSize;
    }

    if (size <= 0)
    {
      logErrorFunction(plugin->GetNameForLogs() + ": error while reading object " + std::string(uuid) + ", size of file is too small: " + boost::lexical_cast<std::string>(fileSize) + " bytes");
      return OrthancPluginErrorCode_StorageAreaPlugin;
    }

    if (OrthancPluginCreateMemoryBuffer64(OrthancPlugins::GetGlobalContext(), target, size) != OrthancPluginErrorCode_Success)
    {
      logErrorFunction(plugin->GetNameForLogs() + ": error while reading object " + std::string(uuid) + ", cannot allocate memory of size " + boost::lexical_cast<std::string>(size) + " bytes");
      return OrthancPluginErrorCode_StorageAreaPlugin;
    }

    if (cryptoEnabled)
    {
      std::vector<char> encrypted(fileSize);
      reader->ReadWhole(encrypted.data(), fileSize);

      try
      {
        crypto->Decrypt(reinterpret_cast<char*>(target->data), encrypted.data(), fileSize);
      }
      catch (EncryptionException& ex)
      {
        logErrorFunction(plugin->GetNameForLogs() + ": error while decrypting object " + std::string(uuid) + ": " + ex.what());
        return OrthancPluginErrorCode_StorageAreaPlugin;
      }
    }
    else
    {
      reader->ReadWhole(reinterpret_cast<char*>(target->data), fileSize);
    }
  }
  catch (StoragePluginException& ex)
  {
    logErrorFunction(plugin->GetNameForLogs() + ": error while decrypting object " + std::string(uuid) + ": " + ex.what());
    return OrthancPluginErrorCode_StorageAreaPlugin;
  }

  return OrthancPluginErrorCode_Success;
}

static OrthancPluginErrorCode StorageReadWhole(OrthancPluginMemoryBuffer64* target, // Memory buffer where to store the content of the file. It must be allocated by the plugin using OrthancPluginCreateMemoryBuffer64(). The core of Orthanc will free it.
                                               const char* uuid,
                                               OrthancPluginContentType type)
{
  OrthancPluginErrorCode res = StorageReadWhole(primaryPlugin.get(),
                                                (IsHybridModeEnabled() ? OrthancPlugins::LogWarning : OrthancPlugins::LogError), // log errors as warning on first try
                                                target,
                                                uuid,
                                                type);

  if (res != OrthancPluginErrorCode_Success && IsHybridModeEnabled())
  {
    res = StorageReadWhole(secondaryPlugin.get(),
                           OrthancPlugins::LogError, // log errors as errors on second try
                           target,
                           uuid,
                           type);
  }
  return res;
}

static OrthancPluginErrorCode StorageReadWholeLegacy(void** content,
                                                     int64_t* size,
                                                     const char* uuid,
                                                     OrthancPluginContentType type)
{
  OrthancPluginMemoryBuffer64 buffer;
  OrthancPluginErrorCode result = StorageReadWhole(&buffer, uuid, type); // will allocate OrthancPluginMemoryBuffer64

  if (result == OrthancPluginErrorCode_Success)
  {
    *size = buffer.size;
    *content = buffer.data; // orthanc will free the buffer (we don't have to delete it ourselves)
  }

  return result;
}


// static bool StorageRemoveFromDisk(const char* uuid,
//                                   OrthancPluginContentType type)
// {
//   try
//   {
//     namespace fs = boost::filesystem;
//     bool fileExisted = false;
//     fs::path path = BaseStoragePlugin::GetOrthancFileSystemPath(uuid, fileSystemRootPath);

//     try
//     {
//       fs::remove(path);
//       fileExisted = true;
//     }
//     catch (...)
//     {
//       // Ignore the error
//       fileExisted = false;
//     }

//     // Remove the two parent directories, ignoring the error code if
//     // these directories are not empty

//     try
//     {
//       boost::system::error_code err;
//       fs::remove(path.parent_path(), err);
//       fs::remove(path.parent_path().parent_path(), err);
//     }
//     catch (...)
//     {
//       // Ignore the error
//     }

//     return fileExisted;
//   }
//   catch(Orthanc::OrthancException& e)
//   {
//     OrthancPlugins::LogError(std::string(StoragePluginFactory::GetStoragePluginName()) + ": error while deleting object " + std::string(uuid) + ": " + std::string(e.What()));
//     return false;
//   }

// }


static OrthancPluginErrorCode StorageRemove(IStoragePlugin* plugin,
                                            LogErrorFunction logErrorFunction,
                                            const char* uuid,
                                            OrthancPluginContentType type)
{
  try
  {
    OrthancPlugins::LogInfo(plugin->GetNameForLogs() + ": deleting attachment " + std::string(uuid) + " of type " + boost::lexical_cast<std::string>(type));
    plugin->DeleteObject(uuid, type, cryptoEnabled);
    if ((plugin == primaryPlugin.get()) && IsHybridModeEnabled())
    {
      // not 100% sure the file has been deleted, try the secondary plugin
      return OrthancPluginErrorCode_StorageAreaPlugin; 
    }
    
    return OrthancPluginErrorCode_Success;
  }
  catch (StoragePluginException& ex)
  {
    logErrorFunction(std::string(StoragePluginFactory::GetStoragePluginName()) + ": error while deleting object " + std::string(uuid) + ": " + std::string(ex.what()));
    return OrthancPluginErrorCode_StorageAreaPlugin;
  }
}

static OrthancPluginErrorCode StorageRemove(const char* uuid,
                                            OrthancPluginContentType type)
{
  OrthancPluginErrorCode res = StorageRemove(primaryPlugin.get(),
                                             (IsHybridModeEnabled() ? OrthancPlugins::LogWarning : OrthancPlugins::LogError), // log errors as warning on first try
                                             uuid,
                                             type);

  if (res != OrthancPluginErrorCode_Success && IsHybridModeEnabled())
  {
    res = StorageRemove(secondaryPlugin.get(),
                        OrthancPlugins::LogError, // log errors as errors on second try
                        uuid,
                        type);
  }
  return res;
}


extern "C"
{
  ORTHANC_PLUGINS_API int32_t OrthancPluginInitialize(OrthancPluginContext* context)
  {
    OrthancPlugins::SetGlobalContext(context);

    Orthanc::InitializeFramework("", false);
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
      const char* pluginSectionName = StoragePluginFactory::GetConfigurationSectionName();
      static const char* const ENCRYPTION_SECTION = "StorageEncryption";

      if (orthancConfig.IsSection(pluginSectionName))
      {
        OrthancPlugins::OrthancConfiguration pluginSection;
        orthancConfig.GetSection(pluginSection, pluginSectionName);

        bool migrationFromFileSystemEnabled = pluginSection.GetBooleanValue("MigrationFromFileSystemEnabled", false);
        std::string hybridModeString = pluginSection.GetStringValue("HybridMode", "Disabled");

        if (migrationFromFileSystemEnabled && hybridModeString == "Disabled")
        {
          hybridMode = HybridMode_WriteToObjectStorage;
          OrthancPlugins::LogWarning(std::string(StoragePluginFactory::GetStoragePluginName()) + ": 'MigrationFromFileSystemEnabled' configuration is deprecated, use 'HybridMode': 'WriteToObjectStorage' instead");
        }
        else if (hybridModeString == "WriteToObjectStorage")
        {
          hybridMode = HybridMode_WriteToObjectStorage;
          OrthancPlugins::LogWarning(std::string(StoragePluginFactory::GetStoragePluginName()) + ": WriteToObjectStorage HybridMode is enabled: writing to object-storage, try reading first from object-storage and, then, from file system");
        }
        else if (hybridModeString == "WriteToFileSystem")
        {
          hybridMode = HybridMode_WriteToFileSystem;
          OrthancPlugins::LogWarning(std::string(StoragePluginFactory::GetStoragePluginName()) + ": WriteToFileSystem HybridMode is enabled: writing to file system, try reading first from file system and, then, from object-storage");
        }
        else
        {
          OrthancPlugins::LogWarning(std::string(StoragePluginFactory::GetStoragePluginName()) + ": HybridMode is disabled enabled: writing to object-storage and reading only from object-storage");
        }

        if (IsReadFromDisk())
        {
          fileSystemRootPath = orthancConfig.GetStringValue("StorageDirectory", "OrthancStorageNotDefined");
          OrthancPlugins::LogWarning(std::string(StoragePluginFactory::GetStoragePluginName()) + ": HybridMode: reading from file system is enabled, source: " + fileSystemRootPath);
        }

        objectsRootPath = pluginSection.GetStringValue("RootPath", std::string());

        if (objectsRootPath.size() >= 1 && objectsRootPath[0] == '/')
        {
          OrthancPlugins::LogError(std::string(StoragePluginFactory::GetStoragePluginName()) + ": The RootPath shall not start with a '/': " + objectsRootPath);
          return -1;
        }

        std::string objecstStoragePluginName = StoragePluginFactory::GetStoragePluginName();
        if (hybridMode == HybridMode_WriteToFileSystem)
        {
          objecstStoragePluginName += " (Secondary: object-storage)";
        }
        else if (hybridMode == HybridMode_WriteToObjectStorage)
        {
          objecstStoragePluginName += " (Primary: object-storage)";
        }

        std::unique_ptr<IStoragePlugin> objectStoragePlugin(StoragePluginFactory::CreateStoragePlugin(objecstStoragePluginName, orthancConfig));

        if (objectStoragePlugin.get() == nullptr)
        {
          return -1;
        }

        objectStoragePlugin->SetRootPath(objectsRootPath);

        std::unique_ptr<IStoragePlugin> fileSystemStoragePlugin;
        if (IsHybridModeEnabled())
        {
          bool fsync = orthancConfig.GetBooleanValue("SyncStorageArea", true);

          std::string filesystemStoragePluginName = StoragePluginFactory::GetStoragePluginName();
          if (hybridMode == HybridMode_WriteToFileSystem)
          {
            filesystemStoragePluginName += " (Primary: file-system)";
          }
          else if (hybridMode == HybridMode_WriteToObjectStorage)
          {
            filesystemStoragePluginName += " (Secondary: file-system)";
          }

          fileSystemStoragePlugin.reset(new FileSystemStoragePlugin(filesystemStoragePluginName, fileSystemRootPath, fsync));
        }

        if (hybridMode == HybridMode_Disabled || hybridMode == HybridMode_WriteToObjectStorage)
        {
          primaryPlugin.reset(objectStoragePlugin.release());
          
          if (hybridMode == HybridMode_WriteToObjectStorage)
          {
            secondaryPlugin.reset(fileSystemStoragePlugin.release());
          }
        }
        else if (hybridMode == HybridMode_WriteToFileSystem)
        {
          primaryPlugin.reset(fileSystemStoragePlugin.release());
          secondaryPlugin.reset(objectStoragePlugin.release());
        }

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

      if (cryptoEnabled)
      {
        // with encrypted file, we do not want to support ReadRange.  Therefore, we register the old interface
        OrthancPluginRegisterStorageArea(context, StorageCreate, StorageReadWholeLegacy, StorageRemove);
      }
      else
      {
        OrthancPluginRegisterStorageArea2(context, StorageCreate, StorageReadWhole, StorageReadRange, StorageRemove);
      }
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
    primaryPlugin.reset();
    secondaryPlugin.reset();
    Orthanc::FinalizeFramework();
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

