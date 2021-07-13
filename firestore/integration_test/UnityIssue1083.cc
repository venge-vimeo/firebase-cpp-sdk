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
#include <string>
#include <thread>

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

void ReportDocumentSnapshotCallback(const DocumentSnapshot& snapshot, Error error, const std::string& error_message) {
  if (error != Error::kErrorOk) {
    std::cout << "DocumentSnapshotCallback() error=" << error << " message: " << error_message << std::endl;
  } else {
    std::cout << "DocumentSnapshotCallback() id=" << snapshot.id()
      << " is_from_cache=" << snapshot.metadata().is_from_cache()
      << " has_pending_writes=" << snapshot.metadata().has_pending_writes()
      << " TestKey=" << snapshot.Get("TestKey", DocumentSnapshot::ServerTimestampBehavior::kNone).string_value()
      << std::endl;
  }
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
  auth->SignOut();

  std::cout << "Auth::current_user()" << std::endl;
  User* user = auth->current_user();
  if (user) {
    std::cout << "Auth::current_user() returned non-null; skipping login" << std::endl;
  } else {
    std::cout << "Auth::current_user() returned null; logging in anonymously" << std::endl;
    auto future = auth->SignInAnonymously();
    AwaitSuccess(future, "SignInAnonymously()");
  }

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

void step2(const std::string& document_path) {
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

  ListenerRegistration listener_registration = doc.AddSnapshotListener(
      MetadataChanges::kInclude, ReportDocumentSnapshotCallback);

  std::this_thread::sleep_for(std::chrono::seconds(4));

  listener_registration.Remove();
}

struct ParsedArgs {
  bool ok;
  std::string document_path;
};

ParsedArgs ParseArgs(int argc, char** argv) {
  ParsedArgs parsed_args;

  if (argc == 1) {
    std::cerr << "ERROR: invalid command-line arguments: "
      << "a document path must be specified" << std::endl;
    parsed_args.ok = false;
  } else if (argc > 2) {
    std::cerr << "ERROR: invalid command-line arguments: "
      << "unexpected argument: " << argv[2] << std::endl;
    parsed_args.ok = false;
  } else {
    parsed_args.ok = true;
  }

  if (parsed_args.ok) {
    parsed_args.document_path = argv[1];
  }

  if (! parsed_args.ok) {
    std::cerr << "Syntax: " << argv[0] << " [document_path]" << std::endl;
    std::cerr << "Example: " << argv[0] << " states/California" << std::endl;
  }

  return parsed_args;
}

int main(int argc, char** argv) {
  ParsedArgs parsed_args = ParseArgs(argc, argv);
  if (! parsed_args.ok) {
    return 2;
  }

  firebase::SetLogLevel(firebase::LogLevel::kLogLevelDebug);

  step1(parsed_args.document_path);
  step2(parsed_args.document_path);

  return 0;
}
