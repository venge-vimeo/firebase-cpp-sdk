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

#include "gmock/gmock.h"
#include "util/assert.h"
#include "util/future_test_util.h"

namespace firebase {
namespace firestore {
namespace testing {

FirestoreInstanceFactory::~FirestoreInstanceFactory() {
  if (firestore_) {
    EXPECT_THAT(firestore_->Terminate(), FutureSucceeds());
    EXPECT_THAT(firestore_->ClearPersistence(), FutureSucceeds());
  }
}

Firestore* FirestoreInstanceFactory::GetFirestore(const std::string& name) {
  FIRESTORE_TESTING_ASSERT_MESSAGE(name == kDefaultAppName,
    "GetFirestore() was invoked with an unsupported name: %s "
    "(it is required to be %s)",
    name.c_str(), kDefaultAppName);
  
  std::lock_guard<std::mutex> lock(mutex_);

  FIRESTORE_TESTING_ASSERT_MESSAGE(! firestore_,
    "GetFirestore() was invoked a subsequent time, "
    "but is only supported to be invoked at most once");

  return nullptr;
}

void FirestoreInstanceFactory::Delete(Firestore* firestore) {
  FIRESTORE_TESTING_ASSERT_MESSAGE(false, "This method is not supported");
}

void FirestoreInstanceFactory::Delete(App* firestore) {
  FIRESTORE_TESTING_ASSERT_MESSAGE(false, "This method is not supported");
}

void FirestoreInstanceFactory::Disown(Firestore* firestore) {
  FIRESTORE_TESTING_ASSERT_MESSAGE(false, "This method is not supported");
}

}  // namespace testing
}  // namespace firestore
}  // namespace firebase
