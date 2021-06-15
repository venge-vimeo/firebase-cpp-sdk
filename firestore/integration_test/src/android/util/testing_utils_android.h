#ifndef FIRESTORE_INTEGRATION_TEST_SRC_ANDROID_UTIL_TESTING_UTILS_ANDROID_H_
#define FIRESTORE_INTEGRATION_TEST_SRC_ANDROID_UTIL_TESTING_UTILS_ANDROID_H_

#include <functional>

#include <jni.h>

namespace firebase {
namespace firestore {
namespace testing {

class AndroidTestUtils {
 public:
  jlong GetCurrentJavaThreadId();
  jlong GetMainJavaThreadId();
  jlong GetAnotherJavaThreadId();
  void RunOnMainThread(std::function<void()>);
};

}  // namespace testing
}  // namespace firestore
}  // namespace firebase

#endif  // FIRESTORE_INTEGRATION_TEST_SRC_ANDROID_UTIL_TESTING_UTILS_ANDROID_H_
