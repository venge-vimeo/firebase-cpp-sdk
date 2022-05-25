/*
 * Copyright 2022 Google LLC
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

#include "firestore/src/include/firebase/firestore/transaction_options.h"

#include <cstdint>
#include <string>

#include "firestore/src/common/hard_assert_common.h"

namespace firebase {
namespace firestore {

void TransactionOptions::set_max_attempts(int32_t max_attempts) {
  SIMPLE_HARD_ASSERT(max_attempts > 0,
                     "invalid max_attempts: " + std::to_string(max_attempts));
  max_attempts_ = max_attempts;
}

std::string TransactionOptions::ToString() const {
  return std::string("TransactionOptions(max_attempts=") + std::to_string(max_attempts()) + ")";
}

std::ostream& operator<<(std::ostream& out, const TransactionOptions& options) {
  return out << options.ToString();
}


}  // namespace firestore
}  // namespace firebase
