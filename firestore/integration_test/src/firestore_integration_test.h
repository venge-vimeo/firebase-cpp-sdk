#ifndef FIREBASE_FIRESTORE_CLIENT_CPP_SRC_TESTS_FIRESTORE_INTEGRATION_TEST_H_
#define FIREBASE_FIRESTORE_CLIENT_CPP_SRC_TESTS_FIRESTORE_INTEGRATION_TEST_H_

#include "firebase/firestore.h"
#include "gtest/gtest.h"

namespace firebase {
namespace firestore {

class FirestoreIntegrationTest : public testing::Test {

 public:
  FirestoreIntegrationTest() = default;

  // Delete the copy and move constructors and assignment operators since
  // instances of this class should never be moved or copied.
  FirestoreIntegrationTest(const FirestoreIntegrationTest&) = delete;
  FirestoreIntegrationTest(FirestoreIntegrationTest&&) = delete;
  FirestoreIntegrationTest& operator=(const FirestoreIntegrationTest&) = delete;
  FirestoreIntegrationTest& operator=(FirestoreIntegrationTest&&) = delete;

  Firestore* TestFirestore(const std::string& app_name = kDefaultAppName);
};

}  // namespace firestore
}  // namespace firebase

#endif  // FIREBASE_FIRESTORE_CLIENT_CPP_SRC_TESTS_FIRESTORE_INTEGRATION_TEST_H_
