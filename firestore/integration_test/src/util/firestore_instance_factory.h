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

#ifndef FIREBASE_FIRESTORE_CLIENT_CPP_SRC_TESTS_UTIL_FIRESTORE_INSTANCE_FACTORY_H_
#define FIREBASE_FIRESTORE_CLIENT_CPP_SRC_TESTS_UTIL_FIRESTORE_INSTANCE_FACTORY_H_

#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include "firebase/auth.h"
#include "firebase/firestore.h"

namespace firebase {
namespace firestore {
namespace testing {

class FirebaseAppFactory {
 public:
  FirebaseAppFactory() = default;

  // Delete the copy and move constructors and assignment operators.
  FirebaseAppFactory(const FirebaseAppFactory&) = delete;
  FirebaseAppFactory& operator=(const FirebaseAppFactory&) = delete;

  App* GetInstance(const std::string& name);
  void SignIn(App* app);
  void SignOut(App* app);
  void SignOutAllApps();

 private:
  App* GetInstanceLocked(const std::string& name);
  void SignOutLocked(App* app);
  void AssertKnownApp(App*);

  std::mutex mutex_;
  std::unordered_map<std::string, std::unique_ptr<App>> apps_;
  std::unordered_map<App*, std::unique_ptr<::firebase::auth::Auth>> auths_;
};

class FirestoreFactory {
 public:
  explicit FirestoreFactory(FirebaseAppFactory& app_factory);

  // Delete the copy and move constructors and assignment operators.
  FirestoreFactory(const FirestoreFactory&) = delete;
  FirestoreFactory& operator=(const FirestoreFactory&) = delete;

  Firestore* GetInstance(const std::string& name);
  void Delete(Firestore* firestore);
  void Disown(Firestore* firestore);
  
  FirebaseAppFactory& app_factory() {
    return app_factory_;
  }

 private:
  FirebaseAppFactory& app_factory_;
  std::mutex mutex_;
  std::unordered_map<std::string, std::unique_ptr<Firestore>> firestores_;
};

}  // namespace testing
}  // namespace firestore
}  // namespace firebase

#endif  // FIREBASE_FIRESTORE_CLIENT_CPP_SRC_TESTS_UTIL_FIRESTORE_INSTANCE_FACTORY_H_
