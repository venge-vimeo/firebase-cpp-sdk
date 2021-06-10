/*
 * Copyright 2021 Google LLC
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
#include "util/firestore_instance_factory.h"

#include "firebase_test_framework.h"
#include "gmock/gmock.h"
#include "util/assert.h"
#include "util/future_test_util.h"

using ::firebase::auth::Auth;
using ::firebase_test_framework::FirebaseTest;

namespace firebase {
namespace firestore {
namespace testing {

namespace {

std::unique_ptr<App> CreateApp() {
  App* app = App::Create();
  FIRESTORE_TESTING_ASSERT_MESSAGE(app, "App::Create() returned null");
  return std::unique_ptr<App>(app);
}

std::unique_ptr<Auth> CreateAuth(App* app) {
  Auth* auth = nullptr;
  ModuleInitializer initializer;
  auto initialize_auth_future = initializer.Initialize(app, &auth, [](App* app, void* target) {
    InitResult result;
    Auth* get_auth_result = Auth::GetAuth(app, &result);
    *reinterpret_cast<Auth**>(target) = get_auth_result;
    return result;
  });
  FirebaseTest::WaitForCompletion(initialize_auth_future, "Auth::GetAuth()");
  FIRESTORE_TESTING_ASSERT_MESSAGE(initialize_auth_future.error() == 0, "Auth::GetAuth() failed");
  FIRESTORE_TESTING_ASSERT_MESSAGE(auth, "Auth::GetAuth() returned null");
  return std::unique_ptr<Auth>(auth);
}

std::unique_ptr<Firestore> CreateFirestore(App* app) {
  Firestore* firestore = nullptr;
  ModuleInitializer initializer;
  auto initialize_firestore_future = initializer.Initialize(app, &firestore, [](App* app, void* target) {
    InitResult result;
    Firestore* get_instance_result = Firestore::GetInstance(app, &result);
    *reinterpret_cast<Firestore**>(target) = get_instance_result;
    return result;
  });
  FirebaseTest::WaitForCompletion(initialize_firestore_future, "Firestore::GetInstance()");
  FIRESTORE_TESTING_ASSERT_MESSAGE(initialize_firestore_future.error() == 0, "Firestore::GetInstance() failed");
  FIRESTORE_TESTING_ASSERT_MESSAGE(firestore, "Firestore::GetInstance() returned null");
  return std::unique_ptr<Firestore>(firestore);
}

}  // namespace

App* FirebaseAppFactory::GetInstance(const std::string& name) {
  SCOPED_TRACE("FirebaseAppFactory::GetApp(" + name + ")");

  FIRESTORE_TESTING_ASSERT_MESSAGE(name == kDefaultAppName,
    "GetApp() was invoked with an unsupported name: %s "
    "(it is required to be %s)",
    name.c_str(), kDefaultAppName);
  
  std::lock_guard<std::mutex> lock(mutex_);

  if (app_) {
    return app_.get();
  }

  {
    SCOPED_TRACE("InitializeApp");
    FIRESTORE_TESTING_ASSERT(!app_);
    app_ = CreateApp();
    FIRESTORE_TESTING_ASSERT(app_);
  }

  return app_.get();
}

void FirebaseAppFactory::SignIn(App* app) {
  std::lock_guard<std::mutex> lock(mutex_);

  FIRESTORE_TESTING_ASSERT_MESSAGE(app == app_.get(), "An unexpected Firebase app instance was specified");

  if (! auth_) {
    SCOPED_TRACE("InitializeAuth");
    FIRESTORE_TESTING_ASSERT(!auth_);
    auth_ = CreateAuth(app_.get());
    FIRESTORE_TESTING_ASSERT(auth_);
  }

  if (auth_->current_user() == nullptr) {
    SCOPED_TRACE("SignIn");
    auto sign_in_future = auth_->SignInAnonymously();
    FirebaseTest::WaitForCompletion(sign_in_future, "Auth::SignInAnonymously()");
    FIRESTORE_TESTING_ASSERT_MESSAGE(sign_in_future.error() == 0, "Auth::SignInAnonymously() failed");
  }
}

FirestoreFactory::FirestoreFactory(FirebaseAppFactory& app_factory) : app_factory_(app_factory) {
}

Firestore* FirestoreFactory::GetInstance(const std::string& name) {
  SCOPED_TRACE("FirestoreFactory::GetInstance(" + name + ")");

  FIRESTORE_TESTING_ASSERT_MESSAGE(name == kDefaultAppName,
    "GetInstance() was invoked with an unsupported name: %s "
    "(it is required to be %s)",
    name.c_str(), kDefaultAppName);

  std::lock_guard<std::mutex> lock(mutex_);

  if (firestore_) {
    return firestore_.get();
  }

  App* app = app_factory_.GetInstance(name);
  app_factory_.SignIn(app);

  {
    SCOPED_TRACE("InitializeFirestore");
    FIRESTORE_TESTING_ASSERT(!firestore_);
    firestore_ = CreateFirestore(app);
    FIRESTORE_TESTING_ASSERT(firestore_);
  }

  return firestore_.get();
}

void FirestoreFactory::Delete(Firestore* firestore) {
  std::lock_guard<std::mutex> lock(mutex_);
  FIRESTORE_TESTING_ASSERT_MESSAGE(firestore == firestore_.get(), "The given Firestore instance is not known");
  firestore_.reset();
}

void FirestoreFactory::Disown(Firestore* firestore) {
  std::lock_guard<std::mutex> lock(mutex_);
  FIRESTORE_TESTING_ASSERT_MESSAGE(false, "This method is not supported");
}

}  // namespace testing
}  // namespace firestore
}  // namespace firebase
