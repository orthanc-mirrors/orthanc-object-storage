#include "BaseStoragePlugin.h"
#include <boost/filesystem/fstream.hpp>

std::string BaseStoragePlugin::GetOrthancFileSystemPath(const std::string& uuid, const std::string& fileSystemRootPath)
{
  boost::filesystem::path path = fileSystemRootPath;

  path /= std::string(&uuid[0], &uuid[2]);
  path /= std::string(&uuid[2], &uuid[4]);
  path /= uuid;

  path.make_preferred();

  return path.string();
}


std::string BaseStoragePlugin::GetPath(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled)
{
  if (enableLegacyStorageStructure_)
  {
    return GetOrthancFileSystemPath(uuid, std::string()); // there's no "root" path in an object store
  }
  else
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
}

bool BaseStoragePlugin::ReadCommonConfiguration(bool& enableLegacyStorageStructure, const OrthancPlugins::OrthancConfiguration& pluginSection)
{
  std::string storageStructure = pluginSection.GetStringValue("StorageStructure", "flat");
  if (storageStructure == "flat")
  {
    enableLegacyStorageStructure = false;
  }
  else
  {
    enableLegacyStorageStructure = true;
    if (storageStructure != "legacy")
    {
      OrthancPlugins::LogError("ObjectStorage/StorageStructure configuration invalid value: " + storageStructure + ", allowed values are 'flat' and 'legacy'");
      return false;
    }
  }

  return true;
}
