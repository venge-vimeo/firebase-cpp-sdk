#include "firestore/src/android/cpp_pointer_android.h"

#include <functional>

#include "app/src/include/firebase/internal/common.h"
#include "app/src/assert.h"
#include "firestore/src/jni/declaration.h"
#include "firestore/src/jni/env.h"
#include "firestore/src/jni/loader.h"
#include "firestore/src/jni/ownership.h"

namespace firebase {
namespace firestore {

using jni::Constructor;
using jni::Env;
using jni::Local;
using jni::Loader;
using jni::Method;

namespace {

constexpr char kClass[] = PROGUARD_KEEP_CLASS "com/google/firebase/firestore/internal/cpp/CppPointer";

Constructor<CppPointerBase> kConstructor("(J)V");
Method<int64_t> kGetPointer("getPointer", "()J");

}  // namespace

class CppPointerBase::NativeData {
 public:
  NativeData(void* pointer, std::function<void(void*)> deleter) : pointer_(pointer), deleter_(deleter) {
  }

  NativeData(const NativeData&) = delete;
  NativeData& operator=(const NativeData&) = delete;

  ~NativeData() {
    FIREBASE_ASSERT_MESSAGE(!pointer_, "Delete() should have been called before CppPointerBase::~NativeData()");
  }

  void Delete() {
    deleter_(pointer_);
    pointer_ = nullptr;
  }

  void* pointer() {
    return pointer_;
  }

  static void NativeDelete(JNIEnv* env, jclass, jlong pointer_data_jlong) {
    auto* pointer_data = reinterpret_cast<NativeData*>(pointer_data_jlong);
    pointer_data->Delete();
    delete pointer_data;
  }

 private:
  void* pointer_ = nullptr;
  std::function<void(void*)> deleter_;
};

void CppPointerBase::Initialize(Loader& loader) {
  loader.LoadClass(kClass, kConstructor, kGetPointer);
  static const JNINativeMethod natives[] = {{"deleteCppObject", "(J)V", reinterpret_cast<void*>(&NativeData::NativeDelete)}};
  loader.RegisterNatives(natives, FIREBASE_ARRAYSIZE(natives));
}

Local<CppPointerBase> CppPointerBase::Create(Env& env, void* pointer, std::function<void(void*)> deleter) {
  return env.New(kConstructor, reinterpret_cast<jlong>(new NativeData(pointer, deleter)));
}

void* CppPointerBase::GetPointer(Env& env) const {
  NativeData* native_data = GetNativeData(env);
  return native_data ? native_data->pointer() : nullptr;
}

CppPointerBase::NativeData* CppPointerBase::GetNativeData(Env& env) const {
  jlong pointer_jlong = env.Call(*this, kGetPointer);
  return env.ok() ? reinterpret_cast<NativeData*>(pointer_jlong) : nullptr;
}

}  // namespace firestore
}  // namespace firebase
