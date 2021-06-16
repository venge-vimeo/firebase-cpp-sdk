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

#ifndef FIRESTORE_INTEGRATION_TEST_SRC_UTIL_FIREBASE_FACTORIES_H_
#define FIRESTORE_INTEGRATION_TEST_SRC_UTIL_FIREBASE_FACTORIES_H_

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
  FirebaseAppFactory();
  ~FirebaseAppFactory();

  // Delete the copy and move constructors and assignment operators.
  FirebaseAppFactory(const FirebaseAppFactory&) = delete;
  FirebaseAppFactory& operator=(const FirebaseAppFactory&) = delete;

  static FirebaseAppFactory& GetInstance();

  App* GetDefaultApp();
  App* GetApp(const std::string& name);
  ::firebase::auth::Auth* GetAuth(App* app);
  void SignIn(App* app);
  void SignOut(App* app);
  void SignOutAllApps();

 private:
  App* GetAppLocked(const std::string& name);
  ::firebase::auth::Auth* GetAuthLocked(App* app);
  void SignOutLocked(App* app);
  void DieIfAppIsNotKnownLocked(App*);

  std::mutex mutex_;
  std::unordered_map<std::string, std::unique_ptr<App>> apps_;
  std::unordered_map<App*, std::unique_ptr<::firebase::auth::Auth>> auths_;
};

class FirestoreFactory {
 public:
  FirestoreFactory();

  // Delete the copy and move constructors and assignment operators.
  FirestoreFactory(const FirestoreFactory&) = delete;
  FirestoreFactory& operator=(const FirestoreFactory&) = delete;

  Firestore* GetDefaultFirestore();
  Firestore* GetFirestore(const std::string& name);
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

#endif  // FIRESTORE_INTEGRATION_TEST_SRC_UTIL_FIREBASE_FACTORIES_H_
