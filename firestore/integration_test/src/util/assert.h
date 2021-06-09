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

#ifndef FIREBASE_FIRESTORE_CLIENT_CPP_SRC_TESTS_UTIL_ASSERT_H_
#define FIREBASE_FIRESTORE_CLIENT_CPP_SRC_TESTS_UTIL_ASSERT_H_

#include <cstdlib>

#include "app_framework.h"

#define FIRESTORE_TESTING_EXPAND_STRINGIFY_(X) #X
#define FIRESTORE_TESTING_EXPAND_STRINGIFY(X) FIRESTORE_TESTING_EXPAND_STRINGIFY_(X)

#define FIRESTORE_TESTING_ASSERT_MESSAGE_PREFIX \
  __FILE__ "(" FIRESTORE_TESTING_EXPAND_STRINGIFY(__LINE__) "): " \
  "FIRESTORE TESTING ASSERTION FAILED: "

// Assert condition is true, if it's false log an assert with the specified
// expression as a string.
#define FIRESTORE_TESTING_ASSERT_WITH_EXPRESSION(condition, expression) \
  do {                                                                  \
    if (!(condition)) {                                                 \
      ::app_framework::LogError(                                        \
          FIRESTORE_TESTING_ASSERT_MESSAGE_PREFIX                       \
          FIRESTORE_TESTING_EXPAND_STRINGIFY(expression));              \
      std::abort();                                                     \
    }                                                                   \
  } while (false)

// Custom assert() implementation that is not compiled out in release builds.
#define FIRESTORE_TESTING_ASSERT(expression) \
  FIRESTORE_TESTING_ASSERT_WITH_EXPRESSION(expression, expression)

// Assert that returns a value if the log operation doesn't abort the program.
#define FIRESTORE_TESTING_ASSERT_RETURN(return_value, expression)    \
  {                                                                  \
    bool condition = !(!(expression));                               \
    FIRESTORE_TESTING_ASSERT_WITH_EXPRESSION(condition, expression); \
    if (!condition) return (return_value);                           \
  }

// Assert that returns if the log operation doesn't abort the program.
#define FIRESTORE_TESTING_ASSERT_RETURN_VOID(expression)             \
  {                                                                  \
    bool condition = !(!(expression));                               \
    FIRESTORE_TESTING_ASSERT_WITH_EXPRESSION(condition, expression); \
    if (!condition) return;                                          \
  }

// Assert condition is true otherwise display the specified expression,
// message and abort.
#define FIRESTORE_TESTING_ASSERT_MESSAGE_WITH_EXPRESSION(condition, expression, ...) \
  do {                                                     \
    if (!(condition)) {                                    \
      ::app_framework::LogError(                           \
          FIRESTORE_TESTING_ASSERT_MESSAGE_PREFIX          \
          FIRESTORE_TESTING_EXPAND_STRINGIFY(expression)); \
      ::app_framework::LogError(__VA_ARGS__);              \
      std::abort();                                        \
    }                                                      \
  } while (false)

// Assert expression is true otherwise display the specified message and
// abort.
#define FIRESTORE_TESTING_ASSERT_MESSAGE(expression, ...) \
  FIRESTORE_TESTING_ASSERT_MESSAGE_WITH_EXPRESSION(       \
  expression, expression, __VA_ARGS__)

// Assert expression is true otherwise display the specified message and
// abort or return a value if the log operation doesn't abort.
#define FIRESTORE_TESTING_ASSERT_MESSAGE_RETURN(return_value, expression, ...) \
  {                                                                            \
    bool condition = !(!(expression));                                         \
    FIRESTORE_TESTING_ASSERT_MESSAGE_WITH_EXPRESSION(condition, expression,    \
                                            __VA_ARGS__);                      \
    if (!condition) return (return_value);                                     \
  }

// Assert expression is true otherwise display the specified message and
// abort or return if the log operation doesn't abort.
#define FIRESTORE_TESTING_ASSERT_MESSAGE_RETURN_VOID(expression, ...)       \
  {                                                                         \
    bool condition = !(!(expression));                                      \
    FIRESTORE_TESTING_ASSERT_MESSAGE_WITH_EXPRESSION(condition, expression, \
                                            __VA_ARGS__);                   \
    if (!condition) return;                                                 \
  }

#endif  // FIREBASE_FIRESTORE_CLIENT_CPP_SRC_TESTS_UTIL_ASSERT_H_
