#include "firestore_integration_test.h"

#include <atomic>

#include "android/util/testing_utils_android.h"

using ::app_framework::GetJniEnv;

namespace firebase {
namespace firestore {
namespace testing {
namespace {

using TestingUtilsAndroidTest = FirestoreIntegrationTest;

TEST_F(TestingUtilsAndroidTest, GetCurrentJavaThreadId) {
  AndroidTestUtils utils;

  auto thread_id1 = utils.GetCurrentJavaThreadId();
  auto thread_id2 = utils.GetCurrentJavaThreadId();

  ASSERT_EQ(thread_id1, thread_id2);
  auto another_thread_id = utils.GetAnotherJavaThreadId();
  ASSERT_NE(thread_id1, another_thread_id);
}

TEST_F(TestingUtilsAndroidTest, GetMainJavaThreadId) {
  AndroidTestUtils utils;

  auto thread_id1 = utils.GetMainJavaThreadId();
  auto thread_id2 = utils.GetMainJavaThreadId();

  ASSERT_EQ(thread_id1, thread_id2);
  auto another_thread_id = utils.GetAnotherJavaThreadId();
  ASSERT_NE(thread_id1, another_thread_id);
}

TEST_F(TestingUtilsAndroidTest, GetAnotherJavaThreadId) {
  AndroidTestUtils utils;

  auto thread_id1 = utils.GetAnotherJavaThreadId();
  auto thread_id2 = utils.GetAnotherJavaThreadId();

  auto current_thread_id = utils.GetCurrentJavaThreadId();
  auto main_thread_id = utils.GetMainJavaThreadId();
  EXPECT_NE(thread_id1, current_thread_id);
  EXPECT_NE(thread_id1, main_thread_id);
  EXPECT_NE(thread_id2, current_thread_id);
  EXPECT_NE(thread_id2, main_thread_id);
}

TEST_F(TestingUtilsAndroidTest, RunOnMainThread) {
  AndroidTestUtils utils;
  std::atomic<bool> callback_invoked;
  callback_invoked = false;
  std::atomic<jlong> captured_thread_id;
  captured_thread_id = -1;
  auto callback = [&callback_invoked, &captured_thread_id, &utils]{
    callback_invoked = true;
    captured_thread_id = utils.GetCurrentJavaThreadId();
  };

  utils.RunOnMainThread(callback);

  ASSERT_EQ(callback_invoked, true);
  auto main_thread_id = utils.GetMainJavaThreadId();
  ASSERT_EQ(captured_thread_id, main_thread_id);
}

}  // namespace
}  // namespace testing
}  // namespace firestore
}  // namespace firebase
