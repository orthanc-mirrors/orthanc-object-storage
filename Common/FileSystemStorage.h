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


#pragma once

#include "IStorage.h"
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;


class FileSystemStoragePlugin: public IStorage
{
public:
  class FileSystemWriter: public IStorage::IWriter
  {
    const fs::path path_;
    bool fsync_;
  public:
    FileSystemWriter(const fs::path& path, bool fsync)
    : path_(path),
      fsync_(fsync)
    {}

    virtual ~FileSystemWriter() {}
    virtual void Write(const char* data, size_t size);
  };

  class FileSystemReader: public IStorage::IReader
  {
    const fs::path path_;
  public:
    FileSystemReader(const fs::path& path)
    : path_(path)
    {}

    virtual ~FileSystemReader() {}
    virtual size_t GetSize();
    virtual void ReadWhole(char* data, size_t size);
    virtual void ReadRange(char* data, size_t size, size_t fromOffset);
  };

  std::string fileSystemRootPath_;
  bool fsync_;
public:
  FileSystemStoragePlugin(const std::string& nameForLogs, const std::string& fileSystemRootPath, bool fsync)
  : IStorage(nameForLogs),
    fileSystemRootPath_(fileSystemRootPath),
    fsync_(fsync)
  {}

  virtual void SetRootPath(const std::string& rootPath) {}

  virtual IStorage::IWriter* GetWriterForObject(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled);
  virtual IStorage::IReader* GetReaderForObject(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled);
  virtual void DeleteObject(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled);
};