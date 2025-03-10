/**
 * Cloud storage plugins for Orthanc
 * Copyright (C) 2020-2023 Osimis S.A., Belgium
 * Copyright (C) 2024-2025 Orthanc Team SRL, Belgium
 * Copyright (C) 2021-2025 Sebastien Jodogne, ICTEAM UCLouvain, Belgium
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


#pragma once

#include "IStorage.h"

#include <Compatibility.h>

#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

class BaseStorage : public IStorage
{
  bool        enableLegacyStorageStructure_;
  std::string rootPath_;

protected:

  BaseStorage(const std::string& nameForLogs, bool enableLegacyStorageStructure):
    IStorage(nameForLogs),
    enableLegacyStorageStructure_(enableLegacyStorageStructure)
  {}

  std::string GetPath(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled, bool useAlternateExtension = false);

public:
  virtual void SetRootPath(const std::string& rootPath) ORTHANC_OVERRIDE
  {
    rootPath_ = rootPath;
  }

  static std::string GetPath(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled, bool legacyFileStructure, const std::string& rootFolder);
  static fs::path GetOrthancFileSystemPath(const std::string& uuid, const std::string& fileSystemRootPath);

  static bool ReadCommonConfiguration(bool& enableLegacyStorageStructure, bool& storageContainsUnknownFiles, const OrthancPlugins::OrthancConfiguration& pluginSection);
};
