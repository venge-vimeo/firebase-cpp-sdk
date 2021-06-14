#ifndef FIREBASE_FIRESTORE_CLIENT_CPP_SRC_TESTS_ANDROID_FIRESTORE_INTEGRATION_TEST_ANDROID_H_
#define FIREBASE_FIRESTORE_CLIENT_CPP_SRC_TESTS_ANDROID_FIRESTORE_INTEGRATION_TEST_ANDROID_H_

#include "firestore_integration_test.h"

#include <jni.h>

#include <functional>

namespace firebase {
namespace firestore {
namespace testing {

jlong GetCurrentJavaThreadId();
jlong GetMainJavaThreadId();
void RunOnMainThread(std::function<void()>);

/** Adds Android-specific functionality to `FirestoreIntegrationTest`. */
class FirestoreAndroidIntegrationTest : public FirestoreIntegrationTest {
};

}  // namespace testing
}  // namespace firestore
}  // namespace firebase

#endif  // FIREBASE_FIRESTORE_CLIENT_CPP_SRC_TESTS_ANDROID_FIRESTORE_INTEGRATION_TEST_ANDROID_H_
