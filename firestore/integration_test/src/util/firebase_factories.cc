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
#include "util/firebase_factories.h"

#include <algorithm>
#include <atomic>

#include "app_framework.h"
#include "firebase_test_framework.h"
#include "gmock/gmock.h"
#include "util/assert.h"
#include "util/future_test_util.h"

using ::app_framework::LogDebug;
using ::firebase::auth::Auth;
using ::firebase::auth::User;
using ::firebase_test_framework::FirebaseTest;

namespace firebase {
namespace firestore {
namespace testing {

namespace {

#if defined(__ANDROID__)

App* PlatformCreateDefaultApp() {
  return App::Create(app_framework::GetJniEnv(), app_framework::GetActivity());
}

App* PlatformCreateAppWithName(const AppOptions& options, const char* name) {
  return App::Create(options, name, app_framework::GetJniEnv(), app_framework::GetActivity());
}

#else  // defined(__ANDROID__)

App* PlatformCreateDefaultApp() {
  LogDebug("zzyzx firebase::App::Create()");
  return App::Create();
}

App* PlatformCreateAppWithName(const AppOptions& options, const char* name) {
  LogDebug("zzyzx firebase::App::Create(%s)", name.c_str());
  return App::Create(options, name);
}

#endif  // defined(__ANDROID__)

std::unique_ptr<App> CreateDefaultApp() {
  App* app = PlatformCreateDefaultApp();
  FIRESTORE_TESTING_ASSERT_MESSAGE(app, "App::Create() returned null");
  return std::unique_ptr<App>(app);
}

std::unique_ptr<App> CreateAppWithName(const std::string& name, const AppOptions& options) {
  FIRESTORE_TESTING_ASSERT(name != kDefaultAppName);
  App* app = PlatformCreateAppWithName(options, name.c_str());
  FIRESTORE_TESTING_ASSERT_MESSAGE(app, "App::Create(%s) returned null", name.c_str());
  return std::unique_ptr<App>(app);
}

std::unique_ptr<Auth> CreateAuth(App* app) {
  Auth* auth = nullptr;
  ModuleInitializer initializer;
  auto initialize_auth_future = initializer.Initialize(app, &auth, [](App* app, void* target) {
    InitResult result;
    LogDebug("zzyzx firebase::auth::Auth::GetAuth()");
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
    LogDebug("zzyzx firebase::firestore::Firestore::GetInstance()");
    Firestore* get_instance_result = Firestore::GetInstance(app, &result);
    *reinterpret_cast<Firestore**>(target) = get_instance_result;
    return result;
  });
  FirebaseTest::WaitForCompletion(initialize_firestore_future, "Firestore::GetInstance()");
  FIRESTORE_TESTING_ASSERT_MESSAGE(initialize_firestore_future.error() == 0, "Firestore::GetInstance() failed");
  FIRESTORE_TESTING_ASSERT_MESSAGE(firestore, "Firestore::GetInstance() returned null");
  return std::unique_ptr<Firestore>(firestore);
}

std::atomic<FirebaseAppFactory*> gFirebaseAppFactorySharedInstance;

}  // namespace

FirebaseAppFactory::FirebaseAppFactory() {
  auto* old_shared_instance = gFirebaseAppFactorySharedInstance.exchange(this);
  FIRESTORE_TESTING_ASSERT(old_shared_instance == nullptr);
}

FirebaseAppFactory::~FirebaseAppFactory() {
  auto* old_shared_instance = gFirebaseAppFactorySharedInstance.exchange(nullptr);
  FIRESTORE_TESTING_ASSERT(old_shared_instance == this);
  LogDebug("zzyzx delete Auth");
  LogDebug("zzyzx delete App");
}

FirebaseAppFactory& FirebaseAppFactory::GetInstance() {
  auto* shared_instance = gFirebaseAppFactorySharedInstance.load();
  FIRESTORE_TESTING_ASSERT(shared_instance != nullptr);
  return *shared_instance;
}

App* FirebaseAppFactory::GetDefaultApp() {
  SCOPED_TRACE("FirebaseAppFactory::GetDefaultApp()");
  return GetApp(kDefaultAppName);
}

App* FirebaseAppFactory::GetApp(const std::string& name) {
  SCOPED_TRACE("FirebaseAppFactory::GetApp()");
  std::lock_guard<std::mutex> lock(mutex_);
  return GetAppLocked(name);
}

App* FirebaseAppFactory::GetAppLocked(const std::string& name) {
  SCOPED_TRACE("FirebaseAppFactory::GetAppLocked()");

  // Return the cached Firebase `App` instance if we have one.
  {
    auto found = apps_.find(name);
    if (found != apps_.end()) {
      return found->second.get();
    }
  }

  // Create the new Firebase `App` instance.
  std::unique_ptr<App> app;
  if (name == kDefaultAppName) {
    SCOPED_TRACE("InitializeDefaultApp");
    app = CreateDefaultApp();
    FIRESTORE_TESTING_ASSERT(app);
  } else {
    const std::string scoped_trace_name = "InitializeApp-" + name;
    SCOPED_TRACE(scoped_trace_name.c_str());
    AppOptions options = GetAppLocked(kDefaultAppName)->options();
    app = CreateAppWithName(name, options);
    FIRESTORE_TESTING_ASSERT(app);
  }

  // Add the newly-created `App` instance to the cache and return it.
  auto emplace_result = apps_.emplace(name, std::move(app));
  FIRESTORE_TESTING_ASSERT(emplace_result.second);
  return emplace_result.first->second.get();
}

Auth* FirebaseAppFactory::GetAuth(App* app) {
  SCOPED_TRACE("FirebaseAppFactory::GetAuth()");
  std::lock_guard<std::mutex> lock(mutex_);
  DieIfAppIsNotKnownLocked(app);
  return GetAuthLocked(app);
}

Auth* FirebaseAppFactory::GetAuthLocked(App* app) {
  SCOPED_TRACE("FirebaseAppFactory::GetAuthLocked()");

  {
    auto found = auths_.find(app);
    if (found != auths_.end()) {
      return found->second.get();
    }
  }

  SCOPED_TRACE("InitializeAuth");
  auto created_auth = CreateAuth(app);
  FIRESTORE_TESTING_ASSERT(created_auth);
  auto emplace_result = auths_.emplace(app, std::move(created_auth));
  FIRESTORE_TESTING_ASSERT(emplace_result.second);
  return emplace_result.first->second.get();
}

void FirebaseAppFactory::SignIn(App* app) {
  SCOPED_TRACE("FirebaseAppFactory::SignIn()");
  std::lock_guard<std::mutex> lock(mutex_);
  DieIfAppIsNotKnownLocked(app);

  Auth* auth = GetAuthLocked(app);

  if (auth->current_user() != nullptr) {
    LogDebug("zzyzx Auth::current_user() returned non-null");
  } else {
    LogDebug("zzyzx Auth::current_user() returned nullptr");
    SCOPED_TRACE("SignIn");
    LogDebug("zzyzx Auth::SignInAnonymously()");
    auto sign_in_future = auth->SignInAnonymously();
    FirebaseTest::WaitForCompletion(sign_in_future, "Auth::SignInAnonymously()");
    FIRESTORE_TESTING_ASSERT_MESSAGE(sign_in_future.error() == 0, "Auth::SignInAnonymously() failed");
    FIRESTORE_TESTING_ASSERT(auth->current_user() != nullptr);
    LogDebug("zzyzx shared_auth_->current_user() returned nullptr");
  }
}

void FirebaseAppFactory::SignOut(App* app) {
  SCOPED_TRACE("FirebaseAppFactory::SignOut()");
  std::lock_guard<std::mutex> lock(mutex_);
  DieIfAppIsNotKnownLocked(app);
  SignOutLocked(app);
}

void FirebaseAppFactory::SignOutLocked(App* app) {
  SCOPED_TRACE("FirebaseAppFactory::SignOutLocked()");

  Auth* auth = nullptr;
  {
    auto found = auths_.find(app);
    if (found == auths_.end()) {
      return;
    }
    auth = found->second.get();
  }

  // Do nothing if there is no user signed in.
  if (! auth) {
    return;
  }

  if (auth->current_user() == nullptr) {
    LogDebug("zzyzx Auth::current_user() returned nullptr");
  }
  LogDebug("zzyzx Auth::current_user() returned non-null");

  // We only handle anonymous logins; if a non-anonymous user is logged in then
  // it must have been done by the test and the test should look after cleaning
  // this up.
  FIRESTORE_TESTING_ASSERT(auth->current_user()->is_anonymous());

  // Delete the anonymous user.
  SCOPED_TRACE("DeleteAnonymousUser");
  LogDebug("zzyzx Auth::current_user()->Delete()");
  auto delete_user_future = auth->current_user()->Delete();
  FirebaseTest::WaitForCompletion(delete_user_future, "Auth::current_user()->Delete()");
  FIRESTORE_TESTING_ASSERT_MESSAGE(delete_user_future.error() == 0, "Auth::current_user()->Delete() failed");
  FIRESTORE_TESTING_ASSERT(auth->current_user() == nullptr);
}

void FirebaseAppFactory::SignOutAllApps() {
  SCOPED_TRACE("FirebaseAppFactory::SignOutAllApps()");
  std::lock_guard<std::mutex> lock(mutex_);
  for (const auto& entry : apps_) {
    SignOutLocked(entry.second.get());
  }
}

void FirebaseAppFactory::DieIfAppIsNotKnownLocked(App* app) {
  for (const auto& entry : apps_) {
    if (entry.second.get() == app) {
      return;
    }
  }
  FIRESTORE_TESTING_DIE_WITH_MESSAGE("The given app is not known");
}

FirestoreFactory::FirestoreFactory() : app_factory_(FirebaseAppFactory::GetInstance()) {
}

FirestoreFactory::~FirestoreFactory() {
  LogDebug("zzyzx delete Firestore");
}

Firestore* FirestoreFactory::GetDefaultFirestore() {
  SCOPED_TRACE("FirestoreFactory::GetDefaultFirestore()");
  return GetFirestore(kDefaultAppName);
}

Firestore* FirestoreFactory::GetFirestore(const std::string& name) {
  SCOPED_TRACE("FirestoreFactory::GetFirestore()");
  std::lock_guard<std::mutex> lock(mutex_);

  // Return the cached `Firestore` instance if we have one.
  {
    auto found = firestores_.find(name);
    if (found != firestores_.end()) {
      return found->second.get();
    }
  }

  // Get or create the Firebase `App` instance to use.
  App* app = app_factory_.GetApp(name);

  // Ensure that we are signed in.
  app_factory_.SignIn(app);

  // Create the new `Firestore` instance.
  std::unique_ptr<Firestore> firestore;
  {
    SCOPED_TRACE("InitializeFirestore");
    firestore = CreateFirestore(app);
    FIRESTORE_TESTING_ASSERT(firestore);
  }

  // Add the newly-created `Firestore` instance to the cache and return it.
  auto emplace_result = firestores_.emplace(name, std::move(firestore));
  FIRESTORE_TESTING_ASSERT(emplace_result.second);
  return emplace_result.first->second.get();
}

void FirestoreFactory::Delete(Firestore* firestore) {
  SCOPED_TRACE("FirestoreFactory::Delete()");
  std::lock_guard<std::mutex> lock(mutex_);

  // Find the given `Firestore` instance in the cache.
  auto it = firestores_.begin();
  while (it != firestores_.end()) {
    if (it->second.get() == firestore) {
      break;
    }
  }

  FIRESTORE_TESTING_ASSERT_MESSAGE(it != firestores_.end(), "The given Firestore instance was not found");

  // Remove the `Firestore` instance from the cache, which will in turn delete
  // it since its `unique_ptr` will look after the deletion.
  firestores_.erase(it);
}

void FirestoreFactory::Disown(Firestore* firestore) {
  SCOPED_TRACE("FirestoreFactory::Disown()");
  std::lock_guard<std::mutex> lock(mutex_);

  // Find the given `Firestore` instance in the cache.
  auto it = firestores_.begin();
  while (it != firestores_.end()) {
    if (it->second.get() == firestore) {
      break;
    }
  }

  FIRESTORE_TESTING_ASSERT_MESSAGE(it != firestores_.end(), "The given Firestore instance was not found");

  // Remove the `Firestore` instance from the cache, but "releasing" it first
  // from the owning `unique_ptr` so that it doesn't get deleted.
  it->second.release();
  firestores_.erase(it);
}

}  // namespace testing
}  // namespace firestore
}  // namespace firebase
