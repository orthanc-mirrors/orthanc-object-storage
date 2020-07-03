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
#include "../Common/EncryptionHelpers.h"
#include "../Common/EncryptionConfigurator.h"

static std::unique_ptr<IStoragePlugin> plugin;

static std::unique_ptr<EncryptionHelpers> crypto;
static bool cryptoEnabled = false;


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
  }
  catch (StoragePluginException& ex)
  {
    OrthancPlugins::LogError(std::string(StoragePluginFactory::GetStoragePluginName()) + ": error while creating object " + std::string(uuid) + ": " + ex.what());
    return OrthancPluginErrorCode_StorageAreaPlugin;
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
    OrthancPlugins::LogError(std::string(StoragePluginFactory::GetStoragePluginName()) + ": error while deleting object " + std::string(uuid) + ": " + ex.what());
    return OrthancPluginErrorCode_StorageAreaPlugin;
  }

  return OrthancPluginErrorCode_Success;
}


extern "C"
{
  ORTHANC_PLUGINS_API int32_t OrthancPluginInitialize(OrthancPluginContext* context)
  {
    OrthancPlugins::SetGlobalContext(context);

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

    plugin.reset(StoragePluginFactory::CreateStoragePlugin(orthancConfig));

    static const char* const ENCRYPTION_SECTION = "StorageEncryption";

    if (orthancConfig.IsSection(ENCRYPTION_SECTION))
    {
      OrthancPlugins::OrthancConfiguration cryptoSection;
      orthancConfig.GetSection(cryptoSection, ENCRYPTION_SECTION);

      crypto.reset(EncryptionConfigurator::CreateEncryptionHelpers(cryptoSection));
      cryptoEnabled = crypto.get() != nullptr;
    }
    else
    {
      OrthancPlugins::LogWarning(std::string(StoragePluginFactory::GetStoragePluginName()) + ": client-side encryption is disabled");
    }

    OrthancPluginRegisterStorageArea(context, StorageCreate, StorageRead, StorageRemove);

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

