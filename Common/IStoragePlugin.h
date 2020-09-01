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

#pragma once

#include <orthanc/OrthancCPlugin.h>
#include <OrthancPluginCppWrapper.h>

class StoragePluginException : public std::runtime_error
{
public:
  StoragePluginException(const std::string& what)
    : std::runtime_error(what)
  {
  }
};




// each "plugin" must also provide these global methods
//class XXXStoragePluginFactory
//{
//public:
//  const char* GetStoragePluginName();
//  IStoragePlugin* CreateStoragePlugin(const OrthancPlugins::OrthancConfiguration& orthancConfig);
//};

class IStoragePlugin
{
public:
  class IWriter
  {
  public:
    IWriter()
    {}

    virtual ~IWriter() {}
    virtual void Write(const char* data, size_t size) = 0;
  };

  class IReader
  {
  public:
    IReader()
    {}

    virtual ~IReader() {}
    virtual size_t GetSize() = 0;
    virtual void Read(char* data, size_t size) = 0;
  };

public:

  virtual IWriter* GetWriterForObject(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled) = 0;
  virtual IReader* GetReaderForObject(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled) = 0;
  virtual void DeleteObject(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled) = 0;
  virtual const char* GetConfigurationSectionName() = 0;
};
