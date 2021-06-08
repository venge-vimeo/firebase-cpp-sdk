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
#include "util/autoid.h"

#include <random>

// The implementation in this file is adapted from
// https://github.com/firebase/firebase-ios-sdk/blob/8187c18e/Firestore/core/src/util/autoid.cc

namespace firebase {
namespace firestore {
namespace testing {

namespace {

const int kAutoIdLength = 20;
const char kAutoIdAlphabet[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

std::random_device& GetSharedRandom() {
  static auto* shared_random = new std::random_device();
  return *shared_random;
}

}  // namespace

std::string CreateAutoId() {
  std::string auto_id;
  auto_id.reserve(kAutoIdLength);

  // -2 here because:
  //   * sizeof(kAutoIdAlphabet) includes the trailing null terminator
  //   * uniform_int_distribution is inclusive on both sizes
  std::uniform_int_distribution<int> letters(0, sizeof(kAutoIdAlphabet) - 2);

  std::random_device& shared_random = GetSharedRandom();
  for (int i = 0; i < kAutoIdLength; i++) {
    int rand_index = letters(shared_random);
    auto_id.append(1, kAutoIdAlphabet[rand_index]);
  }

  return auto_id;  
}

}  // namespace testing
}  // namespace firestore
}  // namespace firebase
