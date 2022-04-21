/*
 * Copyright 2022 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "app/src/heartbeat/heartbeat_storage_desktop.h"

#include <fstream>

#include "app/logged_heartbeats_generated.h"
#include "app/src/filesystem.h"
#include "app/src/include/firebase/internal/mutex.h"

namespace firebase {
namespace heartbeat {

using LoggedHeartbeatsFbs = com::google::firebase::cpp::heartbeat::LoggedHeartbeats;
using com::google::firebase::cpp::heartbeat::UserAgentAndDates;
using com::google::firebase::cpp::heartbeat::CreateLoggedHeartbeats;
using com::google::firebase::cpp::heartbeat::CreateUserAgentAndDates;
using com::google::firebase::cpp::heartbeat::GetLoggedHeartbeats;
using com::google::firebase::cpp::heartbeat::VerifyLoggedHeartbeatsBuffer;

// Using an anonymous namespace for file mutex and filename
namespace {

const char kHeartbeatDir[] = "firebase-heartbeat";
const char kHeartbeatFilename[] = "HEARTBEAT_STORAGE";

// Returns the mutex that protects accesses to the storage file.
Mutex& FileMutex() {
  static Mutex* mutex_ = new Mutex(Mutex::kModeNonRecursive);
  return *mutex_;
}

std::string GetFilename(std::string app_id, std::string& error) {
  std::string app_dir =
      AppDataDir((app_id + "/" + kHeartbeatDir).c_str(), /*should_create=*/true, &error);
  if (app_dir.empty()) {
    return "";
  }

  return app_dir + "/" + kHeartbeatFilename;
}

}  // namespace

HeartbeatStorageDesktop::HeartbeatStorageDesktop(std::string app_id)
    : filename_(GetFilename(app_id, error_)) {
  MutexLock lock(FileMutex());

  // Ensure the file exists, otherwise the first attempt to read it would
  // fail.
  std::ofstream file(filename_, std::ios_base::app);

  if (!file) {
    error_ = "Unable to open '" + filename_ + "'.";
  }
}

bool HeartbeatStorageDesktop::Read(LoggedHeartbeats* heartbeats_output_ptr) {
  MutexLock lock(FileMutex());
  error_ = "";

  if (heartbeats_output_ptr == nullptr) {
    error_ = "Cannot read heartbeats to nullptr.";
    return false;
  }
  // Open the file and seek to the end
  std::ifstream file(filename_, std::ios_base::binary | std::ios_base::ate);
  if (!file) {
    error_ = "Unable to open '" + filename_ + "' for reading.";
    return false;
  }
  // Current position in file is buffer size.
  std::streamsize buffer_len = file.tellg();
  // Rewind to start of the file, then read into a buffer.
  file.seekg(0, std::ios_base::beg);
  char buffer[buffer_len];
  file.read(buffer, buffer_len);
  // Verify that the buffer is a valid flatbuffer.
  ::flatbuffers::Verifier verifier(reinterpret_cast<const uint8_t*>(buffer),
                                   buffer_len);
  if (!VerifyLoggedHeartbeatsBuffer(verifier)) {
    error_ =
        "Failed to parse contents of " + filename_ + " as LoggedHeartbeats.";
    return false;
  }
  const LoggedHeartbeatsFbs* heartbeats_fbs = GetLoggedHeartbeats(&buffer);
  *heartbeats_output_ptr = LoggedHeartbeatsFromFbs(heartbeats_fbs);
  return true;
}

bool HeartbeatStorageDesktop::Write(LoggedHeartbeats heartbeats) const {
  MutexLock lock(FileMutex());
  error_ = "";

  // Clear the file before writing.
  std::ofstream file(filename_, std::ios_base::trunc | std::ios_base::binary);
  if (!file) {
    error_ = "Unable to open '" + filename_ + "' for writing.";
    return false;
  }

  flatbuffers::FlatBufferBuilder fbb = LoggedHeartbeatsToFbs(heartbeats);
  file.write((char*)fbb.GetBufferPointer(), fbb.GetSize());

  return !file.fail();
}

LoggedHeartbeats HeartbeatStorageDesktop::LoggedHeartbeatsFromFbs(
  const LoggedHeartbeatsFbs* heartbeats_fbs) const {
  LoggedHeartbeats heartbeats_struct;
  // TODO(almostmatt): verify format of date string
  heartbeats_struct.last_logged_date = heartbeats_fbs->last_logged_date()->str();
  for (auto user_agent_and_dates : *(heartbeats_fbs->heartbeats())) {
    std::string user_agent = user_agent_and_dates->user_agent()->str();
    for (auto date : *(user_agent_and_dates->dates())) {
      // TODO(almostmatt): verify format of date string
      heartbeats_struct.heartbeats[user_agent].push_back(date->str());
    }
  }
  return heartbeats_struct;
}

flatbuffers::FlatBufferBuilder HeartbeatStorageDesktop::LoggedHeartbeatsToFbs(
  LoggedHeartbeats heartbeats_struct) const {
  flatbuffers::FlatBufferBuilder builder;
  auto last_logged_date = builder.CreateString(heartbeats_struct.last_logged_date);

  std::vector<flatbuffers::Offset<UserAgentAndDates>> agents_and_dates_vector;
  for (auto const& entry : heartbeats_struct.heartbeats) {
    auto user_agent = builder.CreateString(entry.first);
    std::vector<flatbuffers::Offset<flatbuffers::String>> dates_vector;
    for (auto date : entry.second) {
      dates_vector.push_back(builder.CreateString(date));
    }
    auto agent_and_dates = CreateUserAgentAndDates(
      builder, user_agent, builder.CreateVector(dates_vector)); 
    agents_and_dates_vector.push_back(agent_and_dates);
  }
  auto loggedHeartbeats = CreateLoggedHeartbeats(
    builder, last_logged_date, builder.CreateVector(agents_and_dates_vector));
  builder.Finish(loggedHeartbeats);
  return builder;
}

}  // namespace heartbeat
}  // namespace firebase