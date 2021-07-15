/*
 * Copyright 2021 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
This application was created to reproduce https://github.com/firebase/quickstart-unity/issues/1083.

Although the issue was reported against the Firestore Unity SDK, it is actually an issue in the
underlying Firestore C++ SDK, as demonstrated by this application.

Steps to Reproduce:
1. Build and this application.
2. Copy google-services.json into the current directory.
3. Run the application in the directory containing google-services.json.
4. The application should complete successfully.

Expected Results:
The final listener notifications show TestKey=NewValue.

Actual Results:
The final listener notifications show TestKey=OldValue.
*/

#include <chrono>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <mutex>
#include <random>
#include <string>
#include <thread>
#include <vector>
#include <unordered_set>

#include "firebase/auth.h"
#include "firebase/firestore.h"

using ::firebase::App;
using ::firebase::Future;
using ::firebase::FutureBase;
using ::firebase::FutureStatus;
using ::firebase::auth::Auth;
using ::firebase::auth::User;
using ::firebase::firestore::DocumentReference;
using ::firebase::firestore::DocumentSnapshot;
using ::firebase::firestore::Error;
using ::firebase::firestore::FieldValue;
using ::firebase::firestore::Firestore;
using ::firebase::firestore::ListenerRegistration;
using ::firebase::firestore::MapFieldValue;
using ::firebase::firestore::MetadataChanges;
using ::firebase::firestore::SetOptions;
using ::firebase::firestore::SnapshotMetadata;

constexpr int kAutoIdLength = 20;
const char kAutoIdAlphabet[] = "ABCDEFGHJKLMNPQRTUVWXYZ0123456789";

std::string GenerateRandomDocumentPath() {
  std::string auto_id;
  auto_id.reserve(kAutoIdLength);

  // -2 here because:
  //   * sizeof(kAutoIdAlphabet) includes the trailing null terminator
  //   * uniform_int_distribution is inclusive on both sizes
  std::uniform_int_distribution<int> letters(0, sizeof(kAutoIdAlphabet) - 2);

  static auto* random_device = new std::random_device();

  for (int i = 0; i < kAutoIdLength; i++) {
    int rand_index = letters(*random_device);
    auto_id.append(1, kAutoIdAlphabet[rand_index]);
  }

  return "UnityIssue1083/" + auto_id;
}

void AwaitSuccess(const FutureBase& future, const std::string& operation) {
  while (future.status() == FutureStatus::kFutureStatusPending) {
    std::this_thread::yield();
  }
  if (future.status() != FutureStatus::kFutureStatusComplete) {
    std::cerr << operation << " failed with status: " << future.status() << std::endl;
    std::terminate();
  }
  if (future.error() != 0) {
    std::cerr << operation << " failed: " << future.error() << " " << future.error_message() << std::endl;
    std::terminate();
  }
}

std::string ReportDocumentSnapshotCallback(const DocumentSnapshot& snapshot, Error error, const std::string& error_message) {
  if (error != Error::kErrorOk) {
    std::cout << "DocumentSnapshotCallback() error=" << error << " message: " << error_message << std::endl;
    return "";
  }

  const std::string value = snapshot.Get("TestKey", DocumentSnapshot::ServerTimestampBehavior::kNone).string_value();
  std::cout << "DocumentSnapshotCallback() id=" << snapshot.id()
    << " is_from_cache=" << snapshot.metadata().is_from_cache()
    << " has_pending_writes=" << snapshot.metadata().has_pending_writes()
    << " TestKey=" << value
    << std::endl;
  
  return value;
};

void step1(const std::string& document_path) {
  std::cout << "===== Step 1 starting with document: " << document_path << std::endl;
  std::cout << "App::Create()" << std::endl;
  std::unique_ptr<App> app(App::Create());
  if (!app) {
    std::cerr << "App::Create() returned null" << std::endl;
    std::terminate();
  }

  std::cout << "Auth::GetAuth()" << std::endl;
  std::unique_ptr<Auth> auth(Auth::GetAuth(app.get()));
  if (!auth) {
    std::cerr << "Auth::GetAuth() returned null" << std::endl;
    std::terminate();
  }

  // Uncommenting the call to `SignOut()` below may fix `SignInAnonymously()` failing with "internal error".
  //AwaitSuccess(auth->current_user()->Delete(), "auth->current_user()->Delete()");
  //auth->SignOut();

  /*
  std::cout << "Auth::current_user()" << std::endl;
  User* user = auth->current_user();
  if (user) {
    std::cout << "Auth::current_user() returned non-null; skipping login" << std::endl;
  } else {
    std::cout << "Auth::current_user() returned null; logging in anonymously" << std::endl;
    auto future = auth->SignInAnonymously();
    AwaitSuccess(future, "SignInAnonymously()");
  }
  */

  std::cout << "Firestore::GetInstance()" << std::endl;
  std::unique_ptr<Firestore> db(Firestore::GetInstance(app.get()));
  if (!db) {
    std::cerr << "Firestore::GetInstance() returned null" << std::endl;
    std::terminate();
  }

  DocumentReference doc = db->Document(document_path.c_str());

  ListenerRegistration listener_registration = doc.AddSnapshotListener(
      MetadataChanges::kInclude, ReportDocumentSnapshotCallback);

  {
    MapFieldValue original_data;
    original_data["TestKey"] = FieldValue::String("OriginalValue");
    SetOptions set_options;
    std::cout << doc.id() << " Set TestKey=OriginalValue" << std::endl;
    auto future = doc.Set(original_data, set_options);
    AwaitSuccess(future, "Set(TestKey=OriginalValue)");
  }

  {
    std::cout << "DisableNetwork()" << std::endl;
    auto future = db->DisableNetwork();
    AwaitSuccess(future, "DisableNetwork()");
  }

  {
    MapFieldValue new_data;
    new_data["TestKey"] = FieldValue::String("NewValue");
    SetOptions set_options;
    std::cout << doc.id() << " Set TestKey=NewValue" << std::endl;
    auto future = doc.Set(new_data, set_options);
    if (future.error() != 0) {
      std::cerr << "Set() failed: " << future.error() << " " << future.error_message() << std::endl;
      std::terminate();
    }
    future.Release();
  }

  std::this_thread::sleep_for(std::chrono::seconds(4));

  listener_registration.Remove();
}

bool step2(const std::string& document_path) {
  std::cout << "===== Step 2 starting with document: " << document_path << std::endl;
  std::cout << "App::Create()" << std::endl;
  std::unique_ptr<App> app(App::Create());
  if (!app) {
    std::cerr << "App::Create() returned null" << std::endl;
    std::terminate();
  }

  // Uncommenting the block below fixes the bug; that is, it causes the snapshot listener to
  // receive notifications that the value of "TestKey" changed to "NewValue".
  /*
  std::cout << "Auth::GetAuth()" << std::endl;
  std::unique_ptr<Auth> auth(Auth::GetAuth(app.get()));
  if (!auth) {
    std::cerr << "Auth::GetAuth() returned null" << std::endl;
    std::terminate();
  }
  */

  std::cout << "Firestore::GetInstance()" << std::endl;
  std::unique_ptr<Firestore> db(Firestore::GetInstance(app.get()));
  if (!db) {
    std::cerr << "Firestore::GetInstance() returned null" << std::endl;
    std::terminate();
  }

  DocumentReference doc = db->Document(document_path.c_str());

  class CallbackValues {
   public:
    void AddValue(const std::string& value) {
      std::lock_guard<std::mutex> lock(mutex_);
      values_.emplace_back(value);
    }
    std::vector<std::string> values() const {
      std::lock_guard<std::mutex> lock(mutex_);
      return values_;
    }
   private:
    mutable std::mutex mutex_;
    std::vector<std::string> values_;
  };

  auto callback_values = std::make_shared<CallbackValues>();

  ListenerRegistration listener_registration = doc.AddSnapshotListener(
      MetadataChanges::kInclude,
      [callback_values](const DocumentSnapshot& snapshot, Error error, const std::string& error_message) {
        std::string value = ReportDocumentSnapshotCallback(snapshot, error, error_message);
        callback_values->AddValue(value);
      }
  );

  std::this_thread::sleep_for(std::chrono::seconds(4));

  listener_registration.Remove();

  std::vector<std::string> values = callback_values->values();
  if (values.size() == 0) {
    std::cout << "TEST FAILED: no callbacks were received" << std::endl;
    return false;
  }

  std::unordered_set<std::string> errors;
  for (const std::string& value : values) {
    if (value != "NewValue") {
      errors.emplace(value);
    }
  }

  if (errors.size() > 0) {
    std::cout << "TEST FAILED: incorrect values were received: ";
    bool first = true;
    for (const std::string& value : errors) {
      if (! first) {
        std::cout << ", ";
      }
      first = false;
      std::cout << value;
    }
    std::cout << std::endl;
    return false;
  }

  std::cout << "TEST PASSED!" << std::endl;
  return true;
}

int main(int argc, char** argv) {
  if (argc > 1) {
    std::cerr << "ERROR: no command-line arguments are supported, "
      "but one was specified: " << argv[1];
    return 2;
  }

  //firebase::SetLogLevel(firebase::LogLevel::kLogLevelDebug);
  //Firestore::set_log_level(firebase::LogLevel::kLogLevelDebug);

  const std::string document_path = GenerateRandomDocumentPath();
  std::cout << "Using document for this test: " << document_path << std::endl;

  step1(document_path);
  bool test_passed = step2(document_path);

  return test_passed ? 0 : 1;
}
