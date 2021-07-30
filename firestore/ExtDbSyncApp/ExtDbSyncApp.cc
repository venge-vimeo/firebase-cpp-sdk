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
using ::firebase::firestore::CollectionReference;
using ::firebase::firestore::DocumentReference;
using ::firebase::firestore::DocumentSnapshot;
using ::firebase::firestore::Error;
using ::firebase::firestore::FieldValue;
using ::firebase::firestore::Firestore;
using ::firebase::firestore::ListenerRegistration;
using ::firebase::firestore::MapFieldValue;
using ::firebase::firestore::MetadataChanges;
using ::firebase::firestore::QuerySnapshot;
using ::firebase::firestore::SetOptions;
using ::firebase::firestore::SnapshotMetadata;

class MyException : public std::exception {
 public:
  MyException(const std::string& message) : message_(message) {
  }

  std::string message() const {
    return message_;
  }

	const char* what() const noexcept {
    return message_.c_str();
  }

 private:
  std::string message_;
};

template <typename T>
void LogInfo(T first) {
  std::cout << first << std::endl;
}

template <typename T, typename ... Args>
void LogInfo(T first, Args ... args) {
  std::cout << first;
  LogInfo(args ...);
}

void AwaitSuccess(const FutureBase& future, const std::string& operation) {
  while (future.status() == FutureStatus::kFutureStatusPending) {
    std::this_thread::yield();
  }
  if (future.status() != FutureStatus::kFutureStatusComplete) {
    throw MyException(operation + " failed with status: " + std::to_string(future.status()));
  }
  if (future.error() != 0) {
    throw MyException(operation + " failed: " + std::to_string(future.error()) + " " + future.error_message());
  }
}

void ReportQuerySnapshotCallback(const QuerySnapshot& snapshot, Error error, const std::string& error_message) {
  if (error != Error::kErrorOk) {
    LogInfo("ReportQuerySnapshotCallback() error=", error, " message: ", error_message);
  } else {
    LogInfo("ReportQuerySnapshotCallback() size=", snapshot.size(),
      " is_from_cache=", snapshot.metadata().is_from_cache(),
      " has_pending_writes=", snapshot.metadata().has_pending_writes());
  }
};

void RunTest() {
  LogInfo("App::Create()");
  std::unique_ptr<App> app(App::Create());
  if (!app) {
    throw MyException("App::Create() returned null");
  }

  LogInfo("Auth::GetAuth()");
  std::unique_ptr<Auth> auth(Auth::GetAuth(app.get()));
  if (!auth) {
    throw MyException("Auth::GetAuth() returned null");
  }

  // Uncommenting the call to `SignOut()` below may fix `SignInAnonymously()` failing with "internal error".
  //AwaitSuccess(auth->current_user()->Delete(), "auth->current_user()->Delete()");
  //auth->SignOut();

  /*
  User* user = auth->current_user();
  if (user) {
    LogInfo("Auth::current_user() returned non-null; skipping login");
  } else {
    LogInfo("Auth::current_user() returned null; logging in anonymously");
    auto future = auth->SignInAnonymously();
    AwaitSuccess(future, "SignInAnonymously()");
  }
  */

  LogInfo("Firestore::GetInstance()");
  std::unique_ptr<Firestore> db(Firestore::GetInstance(app.get()));
  if (!db) {
    throw MyException("Firestore::GetInstance() returned null");
  }

  CollectionReference collection = db->Collection("ExtDbSyncTest/");

  ListenerRegistration listener_registration = collection.AddSnapshotListener(
      MetadataChanges::kInclude, ReportQuerySnapshotCallback);

  std::this_thread::sleep_for(std::chrono::seconds(4));

  listener_registration.Remove();
}

struct ParsedArgs {
  bool debug_logging_enabled = false;
};

ParsedArgs ParseArgs(int argc, char** argv) {
  ParsedArgs args;
  
  for (int i=1; i<argc; i++) {
    const std::string arg(argv[i]);
    if (arg == "--debug") {
      args.debug_logging_enabled = true;
    } else {
      throw MyException("unrecognized argument: " + arg);
    }
  }

  return args;
}

int main(int argc, char** argv) {
  ParsedArgs args;
  try {
    args = ParseArgs(argc, argv);
  } catch (const MyException& e) {
    std::cerr << "ERROR: invalid command-line arguments: " << e.message() << std::endl;
    return 2;
  }

  if (args.debug_logging_enabled) {
    firebase::SetLogLevel(firebase::LogLevel::kLogLevelDebug);
    Firestore::set_log_level(firebase::LogLevel::kLogLevelDebug);
  }

  try {
    RunTest();
  } catch (const MyException& e) {
    std::cerr << "ERROR: " << e.message() << std::endl;
    return 1;
  }

  return 0;
}
