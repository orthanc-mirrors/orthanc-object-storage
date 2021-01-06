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

#include "../Common/BaseStoragePlugin.h"

#include "google/cloud/storage/client.h"

class GoogleStoragePluginFactory
{
public:
  static const char* GetStoragePluginName();
  static IStoragePlugin* CreateStoragePlugin(const OrthancPlugins::OrthancConfiguration& orthancConfig);
};


class GoogleStoragePlugin : public BaseStoragePlugin
{
public:

  std::string         bucketName_;
  google::cloud::storage::Client mainClient_; // the client that is created at startup.  Each thread should copy it when it needs it. (from the doc: Instances of this class created via copy-construction or copy-assignment share the underlying pool of connections. Access to these copies via multiple threads is guaranteed to work. Two threads operating on the same instance of this class is not guaranteed to work.)

public:

  GoogleStoragePlugin(const std::string& bucketName,
                      google::cloud::storage::Client& mainClient,
                      bool enableLegacyStorageStructure
                      );

  virtual IWriter* GetWriterForObject(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled);
  virtual IReader* GetReaderForObject(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled);
  virtual void DeleteObject(const char* uuid, OrthancPluginContentType type, bool encryptionEnabled);
  virtual const char* GetConfigurationSectionName();
};
