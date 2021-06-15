#include "android/util/testing_utils_android.h"

#include <string>

#include "app_framework.h"
#include "gtest/gtest.h"
#include "util/assert.h"

using ::app_framework::FindClass;
using ::app_framework::GetActivity;
using ::app_framework::GetJniEnv;
using ::app_framework::ProcessEvents;

namespace firebase {
namespace firestore {
namespace testing {

namespace {

const std::string kTestingUtilsClass = "com/google/firebase/firestore/internal/cpp/testing/TestingUtils";
const std::string kNativeRunnableClass = "com/google/firebase/firestore/internal/cpp/testing/TestingUtils$NativeRunnable";
const std::string kGetCurrentThreadIdMethod = "getCurrentThreadId";
const std::string kGetMainThreadIdMethod = "getMainThreadId";
const std::string kGetAnotherJavaThreadIdMethod = "getAnotherThreadId";
const std::string kRunOnMainThreadMethod = "runOnMainThread";

jclass GetNativeRunnableClass(JNIEnv* env) {
  jclass clazz = FindClass(env, GetActivity(), kNativeRunnableClass.c_str());
  FIRESTORE_TESTING_ASSERT(clazz);
  return clazz;
}

jclass GetTestingUtilsClass(JNIEnv* env) {
  jclass clazz = FindClass(env, GetActivity(), kTestingUtilsClass.c_str());
  FIRESTORE_TESTING_ASSERT(clazz);
  return clazz;
}

jlong CallStaticTestingUtilsLongMethod(JNIEnv* env, const std::string& method_name) {
  jclass clazz = GetTestingUtilsClass(env);
  jmethodID method = env->GetStaticMethodID(clazz, method_name.c_str(), "()J");
  jlong return_value = env->CallStaticLongMethod(clazz, method);
  if (env->ExceptionOccurred()) {
    env->ExceptionDescribe();
    std::string error_message = kTestingUtilsClass + "." + method_name + "() failed";
    FIRESTORE_TESTING_DIE_WITH_MESSAGE(error_message.c_str());
  }
  return return_value;
}

class NativeRunData {
 public:
  NativeRunData(std::function<void()> function) : function_(std::move(function))  {
    invoked_ = false;
  }

  void Invoke() {
    function_();
    invoked_ = true;
  }

  bool WaitForInvoked() const {
    for (int i=0; i<50 && !invoked_; i++) {
      ProcessEvents(100);
    }
    return invoked_;
  }

 private:
  std::function<void()> function_;
  std::atomic<bool> invoked_;
};

void nativeRun(JNIEnv* env, jobject, jlong data) {
  auto* native_run_data = reinterpret_cast<NativeRunData*>(data);
  native_run_data->Invoke();
}

const JNINativeMethod gNativeRunnableMethods[] = {
  {"nativeRun", "(J)V", reinterpret_cast<void*>(nativeRun)},
};

}  // namespace

jlong AndroidTestUtils::GetCurrentJavaThreadId() {
  JNIEnv* env = GetJniEnv();
  return CallStaticTestingUtilsLongMethod(env, kGetCurrentThreadIdMethod);
}

jlong AndroidTestUtils::GetMainJavaThreadId() {
  JNIEnv* env = GetJniEnv();
  return CallStaticTestingUtilsLongMethod(env, kGetMainThreadIdMethod);
}

jlong AndroidTestUtils::GetAnotherJavaThreadId() {
  JNIEnv* env = GetJniEnv();
  return CallStaticTestingUtilsLongMethod(env, kGetAnotherJavaThreadIdMethod);
}

void AndroidTestUtils::RunOnMainThread(std::function<void()> function) {
  NativeRunData native_run_data(std::move(function));

  JNIEnv* env = GetJniEnv();
  jclass native_runnable_class = GetNativeRunnableClass(env);
  env->RegisterNatives(native_runnable_class, gNativeRunnableMethods, 1);
  jclass testing_utils_class = GetTestingUtilsClass(env);
  jmethodID method = env->GetStaticMethodID(testing_utils_class, kRunOnMainThreadMethod.c_str(), "(J)V");
  env->CallStaticVoidMethod(testing_utils_class, method, reinterpret_cast<jlong>(&native_run_data));
  if (env->ExceptionOccurred()) {
    env->ExceptionDescribe();
    std::string error_message = kTestingUtilsClass + "." + kRunOnMainThreadMethod + "() failed";
    FIRESTORE_TESTING_DIE_WITH_MESSAGE(error_message.c_str());
  }
  bool invoked = native_run_data.WaitForInvoked();
  EXPECT_EQ(invoked, true);
}

}  // namespace testing
}  // namespace firestore
}  // namespace firebase
