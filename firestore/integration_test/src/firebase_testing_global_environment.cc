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
#include "firebase_testing_global_environment.h"

#include "firebase_test_framework.h"

// The TO_STRING macro is useful for command line defined strings as the quotes
// get stripped.
#define TO_STRING_EXPAND(X) #X
#define TO_STRING(X) TO_STRING_EXPAND(X)

// Path to the Firebase config file to load.
#ifdef FIREBASE_CONFIG
#define FIREBASE_CONFIG_STRING TO_STRING(FIREBASE_CONFIG)
#else
#define FIREBASE_CONFIG_STRING ""
#endif  // FIREBASE_CONFIG

using ::firebase::firestore::testing::FirestoreTestingGlobalState;

namespace firebase_test_framework {

void FirebaseTestingGlobalEnvironment::SetUp() {
  // Look for google-services.json and change the current working directory to
  // the directory that contains it, if found.
  FirebaseTest::FindFirebaseConfig(FIREBASE_CONFIG_STRING);

  global_state_ = new FirestoreTestingGlobalState;
}

void FirebaseTestingGlobalEnvironment::TearDown() {
  delete global_state_;
  global_state_ = nullptr;
}

}  // firebase_test_framework firebase
