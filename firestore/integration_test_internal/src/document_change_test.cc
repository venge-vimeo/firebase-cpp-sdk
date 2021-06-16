#if defined(__ANDROID__)
#include "firestore/src/android/document_change_android.h"
#elif defined(FIRESTORE_STUB_BUILD)
#include "firestore/src/stub/document_change_stub.h"
#endif  // defined(__ANDROID__)

#include "firebase/firestore.h"
#include "firestore/src/common/wrapper_assertions.h"
#include "firestore_integration_test.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace firebase {
namespace firestore {

#if defined(__ANDROID__) || defined(FIRESTORE_STUB_BUILD)

using DocumentChangeTest = FirestoreIntegrationTest;

TEST_F(DocumentChangeTest, Construction) {
  testutil::AssertWrapperConstructionContract<DocumentChange,
                                              DocumentChangeInternal>();
}

TEST_F(DocumentChangeTest, Assignment) {
  testutil::AssertWrapperAssignmentContract<DocumentChange,
                                            DocumentChangeInternal>();
}

#endif  // defined(__ANDROID__) || defined(FIRESTORE_STUB_BUILD)

}  // namespace firestore
}  // namespace firebase
