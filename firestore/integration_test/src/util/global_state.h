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

#ifndef FIREBASE_FIRESTORE_CLIENT_CPP_SRC_TESTS_UTIL_GLOBAL_STATE_H_
#define FIREBASE_FIRESTORE_CLIENT_CPP_SRC_TESTS_UTIL_GLOBAL_STATE_H_

#include <memory>

#include "util/firestore_instance_factory.h"

// Forward declare `FirebaseTestingGlobalEnvironment` for the "friend"
// declaration below.
namespace firebase_test_framework {
class FirebaseTestingGlobalEnvironment;
}  // namespace firebase_test_framework

namespace firebase {
namespace firestore {
namespace testing {

class FirestoreTestingGlobalState {
 public:
  ~FirestoreTestingGlobalState();
  std::unique_ptr<FirestoreFactory> CreateFirestoreFactory();

  static FirestoreTestingGlobalState& GetInstance();

 private:
  friend class ::firebase_test_framework::FirebaseTestingGlobalEnvironment;

  // Instances may only be created by `FirebaseTestingGlobalEnvironment`.
  FirestoreTestingGlobalState();

  std::unique_ptr<FirebaseAppFactory> firebase_app_factory_;
};

}  // namespace testing
}  // namespace firestore
}  // namespace firebase

#endif  // FIREBASE_FIRESTORE_CLIENT_CPP_SRC_TESTS_UTIL_GLOBAL_STATE_H_
