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
    return GetOrthancFileSystemPath(uuid, rootPath_);
  }
  else
  {
    boost::filesystem::path path = rootPath_;
    std::string filename = std::string(uuid);

    if (type == OrthancPluginContentType_Dicom)
    {
      filename += ".dcm";
    }
    else if (type == OrthancPluginContentType_DicomAsJson)
    {
      filename += ".json";
    }
    else if (type == OrthancPluginContentType_DicomUntilPixelData)
    {
      filename += ".dcm.head";
    }
    else
    {
      filename += ".unk";
    }

    if (encryptionEnabled)
    {
      filename += ".enc";
    }
    path /= filename;

    return path.string();
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
