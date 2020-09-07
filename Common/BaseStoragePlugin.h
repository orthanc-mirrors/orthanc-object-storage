#pragma once

#include "IStoragePlugin.h"

class BaseStoragePlugin : public IStoragePlugin
{
  bool        enableLegacyStorageStructure_;
  std::string rootPath_;

protected:

  BaseStoragePlugin(bool enableLegacyStorageStructure):
    enableLegacyStorageStructure_(enableLegacyStorageStructure)
  {}

  std::string GetPath(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled);

public:
  virtual void SetRootPath(const std::string& rootPath)
  {
    rootPath_ = rootPath;
  }

  static std::string GetPath(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled, bool legacyFileStructure, const std::string& rootFolder);
  static std::string GetOrthancFileSystemPath(const std::string& uuid, const std::string& fileSystemRootPath);

  static bool ReadCommonConfiguration(bool& enableLegacyStorageStructure, const OrthancPlugins::OrthancConfiguration& pluginSection);
};
