#include "android/firestore_integration_test_android.h"

#include <atomic>
#include <jni.h>

#include "app_framework.h"

using ::app_framework::GetJniEnv;

namespace firebase {
namespace firestore {
namespace testing {
namespace {

TEST_F(FirestoreAndroidIntegrationTest, RunOnMainThreadShouldRunOnTheMainThread) {
  std::atomic<jlong> thread_id;
  thread_id = -1;
  auto thread_id_capturer = [&thread_id]() {
    thread_id = GetCurrentJavaThreadId();
  };

  RunOnMainThread(thread_id_capturer);

  jlong main_thread_id = GetMainJavaThreadId();
  ASSERT_EQ(thread_id, main_thread_id);
}

}  // namespace
}  // namespace testing
}  // namespace firestore
}  // namespace firebase
