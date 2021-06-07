#ifndef FIREBASE_FIRESTORE_CLIENT_CPP_SRC_TESTS_FIRESTORE_INTEGRATION_TEST_H_
#define FIREBASE_FIRESTORE_CLIENT_CPP_SRC_TESTS_FIRESTORE_INTEGRATION_TEST_H_

#include <memory>

#include "firebase/firestore.h"
#include "firebase_test_framework.h"
#include "gtest/gtest.h"

namespace firebase_testapp_automated {

using firebase_test_framework::FirebaseTest;

class FirestoreIntegrationTest : public FirebaseTest {
 public:
  static void SetUpTestSuite();

 protected:
  std::shared_ptr<::firebase::firestore::Firestore> TestFirestore(const std::string& app_name = ::firebase::kDefaultAppName);

 private:
  bool default_firestore_created_ = false;
};

}  // namespace firebase_testapp_automated

#endif  // FIREBASE_FIRESTORE_CLIENT_CPP_SRC_TESTS_FIRESTORE_INTEGRATION_TEST_H_
