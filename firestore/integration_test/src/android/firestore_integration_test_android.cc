#include "android/firestore_integration_test_android.h"

#include "app_framework.h"

using ::app_framework::GetJniEnv;

namespace firebase {
namespace firestore {
namespace testing {

namespace {

jlong GetThreadId(JNIEnv* env, jobject thread) {
  jclass thread_class = env->FindClass("java/lang/Thread");
  jmethodID get_id_method = env->GetMethodID(thread_class, "getId", "()J");
  jlong thread_id = env->CallLongMethod(thread, get_id_method);
  env->DeleteLocalRef(thread_class);
  return thread_id;
}

jobject GetMainLooper(JNIEnv* env) {
  jclass looper_class = env->FindClass("android/os/Looper");
  jmethodID get_main_looper_method = env->GetStaticMethodID(looper_class, "getMainLooper", "()Landroid/os/Looper;");
  jobject main_looper = env->CallStaticObjectMethod(looper_class, get_main_looper_method);
  env->DeleteLocalRef(looper_class);
  return main_looper;
}

jobject GetLooperThread(JNIEnv* env, jobject looper) {
  jclass looper_class = env->FindClass("android/os/Looper");
  jmethodID get_thread_method = env->GetMethodID(looper_class, "getThread", "()Ljava/lang/Thread;");
  jobject looper_thread = env->CallObjectMethod(looper, get_thread_method);
  env->DeleteLocalRef(looper_class);
  return looper_thread;
}

jobject GetCurrentThread(JNIEnv* env) {
  jclass thread_class = env->FindClass("java/lang/Thread");
  jmethodID current_thread_method = env->GetStaticMethodID(thread_class, "currentThread", "()Ljava/lang/Thread;");
  jobject current_thread = env->CallStaticObjectMethod(thread_class, current_thread_method);
  env->DeleteLocalRef(thread_class);
  return current_thread;
}

jobject GetMainThread(JNIEnv* env) {
  jobject main_looper = GetMainLooper(env);
  jobject main_thread = GetLooperThread(env, main_looper);
  env->DeleteLocalRef(main_looper);
  return main_thread;
}

}  // namespace

jlong GetCurrentJavaThreadId() {
  JNIEnv* env = GetJniEnv();
  jobject current_thread = GetCurrentThread(env);
  jlong current_thread_id = GetThreadId(env, current_thread);
  env->DeleteLocalRef(current_thread);
  return current_thread_id;
}

jlong GetMainJavaThreadId() {
  JNIEnv* env = GetJniEnv();
  jobject main_thread = GetMainThread(env);
  jlong current_thread_id = GetThreadId(env, main_thread);
  env->DeleteLocalRef(main_thread);
  return current_thread_id;
}

void RunOnMainThread(std::function<void()> function) {
  TestFirestore();  // Initialize the JNI environment.
  JNIEnv* env = GetJniEnv();
  jclass jni_runnable_class = env->FindClass("com/google/firebase/firestore/internal/cpp/JniRunnable");
}

}  // namespace testing
}  // namespace firestore
}  // namespace firebase
