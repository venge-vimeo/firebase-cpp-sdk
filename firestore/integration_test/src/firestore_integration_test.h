#ifndef FIREBASE_FIRESTORE_CLIENT_CPP_SRC_TESTS_FIRESTORE_INTEGRATION_TEST_H_
#define FIREBASE_FIRESTORE_CLIENT_CPP_SRC_TESTS_FIRESTORE_INTEGRATION_TEST_H_

#include <string>

#include "firebase/auth.h"
#include "firebase/firestore.h"
#include "firebase_test_framework.h"
#include "gtest/gtest.h"

namespace firebase_testapp_automated {

using firebase_test_framework::FirebaseTest;
using ::firebase::App;
using ::firebase::kDefaultAppName;
using ::firebase::auth::Auth;
using ::firebase::firestore::Firestore;

// Converts a Firestore error code to a human-friendly name. The `error_code`
// argument is expected to be an element from the firebase::firestore::Error
// enum, but this function will gracefully handle the case where it is not.
std::string ToFirestoreErrorCodeName(int error_code);

class FirestoreIntegrationTest : public FirebaseTest {

 public:
  static void SetUpTestSuite();
  void TearDown() override;

 protected:
  App* app();
  Firestore* TestFirestore(const std::string& app_name = kDefaultAppName);

 private:
  bool default_firestore_created_ = false;
  App* app_ = nullptr;
  Auth* auth_ = nullptr;
  Firestore* firestore_ = nullptr;
};

}  // namespace firebase_testapp_automated

#endif  // FIREBASE_FIRESTORE_CLIENT_CPP_SRC_TESTS_FIRESTORE_INTEGRATION_TEST_H_
