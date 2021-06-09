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

#ifndef FIREBASE_FIRESTORE_CLIENT_CPP_SRC_TESTS_UTIL_GTEST_GLOBAL_STATE_H_
#define FIREBASE_FIRESTORE_CLIENT_CPP_SRC_TESTS_UTIL_GTEST_GLOBAL_STATE_H_

#include <memory>

#include "util/firestore_instance_factory.h"
#include "gtest/gtest.h"

namespace firebase {
namespace firestore {
namespace testing {

class Environment : public ::testing::Environment {
 public:
  void SetUp() override;
  void TearDown() override;
  static std::unique_ptr<FirestoreFactory> CreateFirestoreFactory();
};

}  // namespace testing
}  // namespace firestore
}  // namespace firebase

#endif  // FIREBASE_FIRESTORE_CLIENT_CPP_SRC_TESTS_UTIL_GTEST_GLOBAL_STATE_H_
