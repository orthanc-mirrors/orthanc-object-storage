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

#include "MoveStorageJob.h"
#include "Logging.h"


MoveStorageJob::MoveStorageJob(const std::string& targetStorage,
                               const std::vector<std::string>& instances,
                               const Json::Value& resourceForJobContent,
                               bool cryptoEnabled)
  : OrthancPlugins::OrthancJob("MoveStorage"),
    targetStorage_(targetStorage),
    instances_(instances),
    processedInstancesCount_(0),
    resourceForJobContent_(resourceForJobContent),
    fileSystemStorage_(NULL),
    objectStorage_(NULL),
    cryptoEnabled_(cryptoEnabled)
{
}

void MoveStorageJob::SetStorages(IStorage* fileSystemStorage, IStorage* objectStorage)
{
  fileSystemStorage_ = fileSystemStorage;
  objectStorage_ = objectStorage;
}

static bool MoveAttachment(const std::string& uuid, int type, IStorage* sourceStorage, IStorage* targetStorage, bool cryptoEnabled)
{
  std::vector<char> buffer;
  
  // read from source storage
  try
  {
    OrthancPlugins::LogInfo("Move attachment: " + sourceStorage->GetNameForLogs() + ": reading attachment " + std::string(uuid) + " of type " + boost::lexical_cast<std::string>(type));
    std::unique_ptr<IStorage::IReader> reader(sourceStorage->GetReaderForObject(uuid.c_str(), static_cast<OrthancPluginContentType>(type), cryptoEnabled));

    size_t fileSize = reader->GetSize();
    buffer.resize(fileSize);

    reader->ReadWhole(buffer.data(), fileSize);
  }
  catch (StoragePluginException& ex)
  {
    OrthancPlugins::LogInfo("Move attachment: " + sourceStorage->GetNameForLogs() + ": error while reading attachment " + std::string(uuid) + " of type " + boost::lexical_cast<std::string>(type) + ", this likely means that the file is already on the right storage");
    return true;
  }

  // write to target storage
  if (buffer.size() > 0)
  {
    try
    {
      std::unique_ptr<IStorage::IWriter> writer(targetStorage->GetWriterForObject(uuid.c_str(), static_cast<OrthancPluginContentType>(type), cryptoEnabled));

      writer->Write(buffer.data(), buffer.size());
    }
    catch (StoragePluginException& ex)
    {
      OrthancPlugins::LogError("Move attachment: " + targetStorage->GetNameForLogs() + ": error while writing attachment " + std::string(uuid) + " of type " + boost::lexical_cast<std::string>(type) + ": " + ex.what());
      return false;
    }
  }

  // everything went well so fare, we can delete from source storage
  if (buffer.size() > 0)
  {
    try
    {
      sourceStorage->DeleteObject(uuid.c_str(), static_cast<OrthancPluginContentType>(type), cryptoEnabled);
    }
    catch (StoragePluginException& ex)
    {
      OrthancPlugins::LogError("Move attachment: " + sourceStorage->GetNameForLogs() + ": error while deleting attachment " + std::string(uuid) + " of type " + boost::lexical_cast<std::string>(type) + ": " + ex.what());
      return false;
    }
  }
  return true;
}

static bool MoveInstance(const std::string& instanceId, IStorage* sourceStorage, IStorage* targetStorage, bool cryptoEnabled)
{
  LOG(INFO) << "Moving instance from " << sourceStorage->GetNameForLogs() << " to " << targetStorage->GetNameForLogs();

  Json::Value attachmentsList;
  OrthancPlugins::RestApiGet(attachmentsList, std::string("/instances/") + instanceId + "/attachments?full", false);

  Json::Value::Members attachmentsMembers = attachmentsList.getMemberNames();
  bool success = true;

  for (size_t i = 0; i < attachmentsMembers.size(); i++)
  {
    int attachmentId = attachmentsList[attachmentsMembers[i]].asInt();

    Json::Value attachmentInfo;
    OrthancPlugins::RestApiGet(attachmentInfo, std::string("/instances/") + instanceId + "/attachments/" + boost::lexical_cast<std::string>(attachmentId) + "/info", false);

    std::string attachmentUuid = attachmentInfo["Uuid"].asString();

    // now we have the uuid and type.  We actually don't know where the file is but we'll try to move it anyway to the requested target
    success &= MoveAttachment(attachmentUuid, attachmentId, sourceStorage, targetStorage, cryptoEnabled);
  }

  return success;
}

OrthancPluginJobStepStatus MoveStorageJob::Step()
{
  if (processedInstancesCount_ < instances_.size())
  {
    IStorage* sourceStorage = (targetStorage_ == "file-system" ? objectStorage_ : fileSystemStorage_);
    IStorage* targetStorage = (targetStorage_ == "file-system" ? fileSystemStorage_ : objectStorage_);

    if (MoveInstance(instances_[processedInstancesCount_], sourceStorage, targetStorage, cryptoEnabled_))
    {
      processedInstancesCount_++;
      return OrthancPluginJobStepStatus_Continue;
    }
    else
    {
      return OrthancPluginJobStepStatus_Failure;
    }
  }

  return OrthancPluginJobStepStatus_Success;
}

void MoveStorageJob::Stop(OrthancPluginJobStopReason reason)
{
}
    
void MoveStorageJob::Reset()
{
  processedInstancesCount_ = 0;
}
