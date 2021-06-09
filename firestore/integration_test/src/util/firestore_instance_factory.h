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

#include "firebase/auth.h"
#include "firebase/firestore.h"

namespace firebase {
namespace firestore {
namespace testing {

class FirestoreInstanceFactory {
 public:
  FirestoreInstanceFactory() = default;

  // Delete the copy and move constructors and assignment operators.
  FirestoreInstanceFactory(const FirestoreInstanceFactory&) = delete;
  FirestoreInstanceFactory& operator=(const FirestoreInstanceFactory&) = delete;

  App* GetApp(const std::string& name);
  Firestore* GetFirestore(const std::string& name);
  void Delete(Firestore* firestore);
  void Delete(App* app);
  void Disown(Firestore* firestore);

 private:
  std::mutex mutex_;
  std::unique_ptr<App> app_;
  std::unique_ptr<::firebase::auth::Auth> auth_;
  std::unique_ptr<Firestore> firestore_;

};

}  // namespace testing
}  // namespace firestore
}  // namespace firebase

#endif  // FIREBASE_FIRESTORE_CLIENT_CPP_SRC_TESTS_UTIL_FIRESTORE_INSTANCE_FACTORY_H_
