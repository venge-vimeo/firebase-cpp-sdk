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
#include "firestore/src/common/future2.h"
#include "firestore/src/common/future2_internal.h"

#include <ostream>
#include <sstream>
#include <string>
#include <vector>

#include "app/meta/move.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

using ::firebase::Future2;
using ::firebase::Future2Base;
using ::firebase::Future2Completer;
using ::firebase::Future2Status;
using ::firebase::Move;
using ::testing::PrintToString;
using ::testing::MatchResultListener;

// The `future2_test_utils` namespace contains some helper functions that need
// to be accessible by two different namespaces. Ideally, these functions would
// be in an anonymous namespace but doing so prevents sharing them with multiple
// namespaces. As a workaround, just define them in this "test util" namespace
// and add "using" declarations to make them visible unqualified.
namespace firebase {
namespace future2_test_utils {

std::string NameFromFuture2Status(Future2Status status) {
  switch (status) {
    case Future2Status::kFutureStatusInvalid:
      return "kFutureStatusInvalid";
    case Future2Status::kFutureStatusPending:
      return "kFutureStatusPending";
    case Future2Status::kFutureStatusComplete:
      return "kFutureStatusComplete";
    default:
      return std::to_string(static_cast<int>(status));
  }
}

template <typename T>
void PrintFutureTo(std::ostream& os, Future2Status status, int error, std::string error_message, const T* result) {
  os << "Future{"
    << "status=" << NameFromFuture2Status(status)
    << ", error=" << PrintToString(error)
    << ", error_message=" << PrintToString(error_message)
    << ", result=" << (result ? PrintToString(*result) : "nullptr")
    << "}";
}

template <typename T>
void PrintFutureTo(std::ostream& os, const Future2<T>& future) {
  PrintFutureTo(os, future.status(), future.error(), future.error_message(), future.result());
}

template <typename T>
std::string DebugStringFromFuture(Future2Status status, int error, std::string error_message, const T* result) {
  std::ostringstream ss;
  PrintFutureTo(ss, status, error, error_message, result);
  return ss.str();
}

template <typename T>
std::string DebugStringFromFuture(Future2Status status, int error, std::string error_message, const T& result) {
  return DebugStringFromFuture(status, error, error_message, &result);
}

// A convenience specialization of `DebugStringFromFuture` for nullptr.
// Without this specialization the caller would need to explicitly specify void*
// as the template argument, which is just annoying boilerplate.
template <>
std::string DebugStringFromFuture<nullptr_t>(Future2Status status, int error, std::string error_message, const nullptr_t&) {
  return DebugStringFromFuture<void*>(status, error, error_message, nullptr);
}

}  // namespace future2_test_utils
}  // namespace firebase

using firebase::future2_test_utils::DebugStringFromFuture;
using firebase::future2_test_utils::NameFromFuture2Status;
using firebase::future2_test_utils::PrintFutureTo;

////////////////////////////////////////////////////////////////////////////////

// These `PrintTo` functions are used by googletest when "printing" objects
// in info/error messages. They must exist in the same namespace as the objects
namespace firebase {

void PrintTo(const Future2Status& status, std::ostream* os) {
  *os << NameFromFuture2Status(status);
}

template <typename T>
void PrintTo(const Future2<T>& future, std::ostream* os) {
  PrintFutureTo(*os, future);
}

// Explicitly instantiate the `PrintTo` template function with the template
// parameters that we use; otherwise googletest will not find them at runtime
// since a template function doesn't actually exist until it is instantiated.
template void PrintTo(const Future2<int>&, std::ostream*);

}  // namespace firebase

////////////////////////////////////////////////////////////////////////////////

// Put the tests into an anonymous namespace just to keep them isolated.
namespace {

template <typename T>
bool CompareFutureProperties(MatchResultListener& result_listener, const Future2<T>& future, Future2Status expected_status, int expected_error, const std::string& expected_error_message, const T* expected_result) {
  std::vector<std::string> non_matching_property_names;
  if (future.status() != expected_status) {
    non_matching_property_names.push_back("status");
  }
  if (future.error() != expected_error) {
    non_matching_property_names.push_back("error");
  }
  if (future.error_message() != expected_error_message) {
    non_matching_property_names.push_back("error");
  }
  if (future.result() != expected_result && (future.result() == nullptr || expected_result == nullptr || *expected_result != *future.result())) {
    non_matching_property_names.push_back("result");
  }

  if (non_matching_property_names.empty()) {
    return true;
  }

  result_listener << "match failed for: ";
  bool first_non_matching_property_name = true;
  for (const std::string& property_name : non_matching_property_names) {
    if (first_non_matching_property_name) {
      first_non_matching_property_name = false;
    } else {
      result_listener << ", ";
    }
    result_listener << property_name;
  }

  return false;
}

// A specialization of `CompareFutureProperties` that allows the expected result
// to be specified as `nullptr`.
template <typename T>
bool CompareFutureProperties(MatchResultListener& result_listener, const Future2<T>& future, Future2Status expected_status, int expected_error, const std::string& expected_error_message, const nullptr_t&) {
  return CompareFutureProperties<T>(result_listener, future, expected_status, expected_error, expected_error_message, static_cast<T*>(nullptr));
}

// Matcher that verifies that a Future2 instance is "invalid".
MATCHER(FutureIsInvalid, DebugStringFromFuture(Future2Status::kFutureStatusInvalid, -1, "", nullptr)) {
  return CompareFutureProperties(*result_listener, arg, Future2Status::kFutureStatusInvalid, -1, "", nullptr);
}

// Matcher that verifies that a Future2 instance is in the "successful" state
// with the given result. The `expected_result` must be a value type (not a
// pointer type) and its value will be compared by value against the
// dereferenced pointer returned from the Future2 instance's `result()` method.
MATCHER_P(FutureSucceededWithResult, expected_result, DebugStringFromFuture(Future2Status::kFutureStatusComplete, 0, "", expected_result)) {
  return CompareFutureProperties(*result_listener, arg, Future2Status::kFutureStatusComplete, 0, "", &expected_result);
}

// Matcher that verifies that a Future2 instance is in the "failed" state
// with the given error and error message.
MATCHER_P2(FutureFailedWithError, expected_error, expected_error_message, DebugStringFromFuture(Future2Status::kFutureStatusComplete, expected_error, expected_error_message, nullptr)) {
  return CompareFutureProperties(*result_listener, arg, Future2Status::kFutureStatusComplete, expected_error, expected_error_message, nullptr);
}

TEST(Future2Test, ZeroArgumentConstructor) {
  Future2<int> future;

  EXPECT_THAT(future, FutureIsInvalid());
}

TEST(Future2Test, CopyConstructorCopiesInvalidFuture) {
  Future2<int> invalid_future;

  Future2<int> invalid_future_copy(invalid_future);

  EXPECT_THAT(invalid_future_copy, FutureIsInvalid());
}

TEST(Future2Test, CopyConstructorCopiesSuccessfulFuture) {
  Future2<int> successful_future;
  Future2Completer<int> completer(successful_future);
  completer.CompleteSuccessfully(42);

  Future2<int> successful_future_copy(successful_future);

  EXPECT_THAT(successful_future_copy, FutureSucceededWithResult(42));
}

TEST(Future2Test, CopyConstructorCopiesFailedFuture) {
  Future2<int> failed_future;
  Future2Completer<int> completer(failed_future);
  completer.CompleteUnsuccessfully(1234, "errmsg");

  Future2<int> failed_future_copy(failed_future);

  EXPECT_THAT(failed_future_copy, FutureFailedWithError(1234, "errmsg"));
}

TEST(Future2Test, CopyConstructorCopiesAMovedFromFuture) {
  Future2<int> moved_from_future;
  Future2Completer<int> completer(moved_from_future);
  completer.CompleteSuccessfully(42);
  Future2<int>(Move(moved_from_future));

  Future2<int> moved_from_future_copy(moved_from_future);

  EXPECT_THAT(moved_from_future_copy, FutureIsInvalid());
}

TEST(Future2Test, MoveConstructorMovesInvalidFuture) {
  Future2<int> invalid_future;

  Future2<int> invalid_future_moved(Move(invalid_future));

  EXPECT_THAT(invalid_future_moved, FutureIsInvalid());
}

TEST(Future2Test, MoveConstructorMovesSuccessfulFuture) {
  Future2<int> successful_future;
  Future2Completer<int> completer(successful_future);
  completer.CompleteSuccessfully(42);

  Future2<int> successful_future_moved(Move(successful_future));

  EXPECT_THAT(successful_future_moved, FutureSucceededWithResult(42));
}

TEST(Future2Test, MoveConstructorMovesFailedFuture) {
  Future2<int> failed_future;
  Future2Completer<int> completer(failed_future);
  completer.CompleteUnsuccessfully(1234, "errmsg");

  Future2<int> failed_future_moved(Move(failed_future));

  EXPECT_THAT(failed_future_moved, FutureFailedWithError(1234, "errmsg"));
}

TEST(Future2Test, MoveConstructorMovesMovedFromFuture) {
  Future2<int> moved_from_future;
  Future2Completer<int> completer(moved_from_future);
  completer.CompleteSuccessfully(42);
  Future2<int>(Move(moved_from_future));

  Future2<int> moved_from_future_moved(Move(moved_from_future));

  EXPECT_THAT(moved_from_future_moved, FutureIsInvalid());
}

}  // namespace
