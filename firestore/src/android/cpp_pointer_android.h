#ifndef FIREBASE_FIRESTORE_CLIENT_CPP_SRC_ANDROID_CPP_POINTER_ANDROID_H_
#define FIREBASE_FIRESTORE_CLIENT_CPP_SRC_ANDROID_CPP_POINTER_ANDROID_H_

#include <functional>
#include <memory>

#include "firestore/src/jni/declaration.h"
#include "firestore/src/jni/env.h"
#include "firestore/src/jni/object.h"
#include "firestore/src/jni/ownership.h"

namespace firebase {
namespace firestore {

class CppPointerBase : public jni::Object {
 public:
  using Object::Object;

  static void Initialize(jni::Loader& loader);

  static jni::Local<CppPointerBase> Create(jni::Env& env, void* pointer, std::function<void(void*)> deleter);

  void* GetPointer(jni::Env& env) const;

 private:
  class NativeData;
  NativeData* GetNativeData(jni::Env& env) const;
};

template <typename T>
class CppPointer : public CppPointerBase {
 public:
  using CppPointerBase::CppPointerBase;

  static jni::Local<CppPointer> Create(jni::Env& env, std::shared_ptr<T> pointer) {
    return jni::Local<CppPointer>(CppPointer(CppPointerBase::Create(env, new std::shared_ptr<T>(pointer), &Deleter).get()));
  }

  std::shared_ptr<T> GetPointer(jni::Env& env) const {
    return *static_cast<std::shared_ptr<T>*>(CppPointerBase::GetPointer(env));
  }
 
 private:
  static void Deleter(void* pointer) {
    delete reinterpret_cast<std::shared_ptr<T>*>(pointer);
  }
};

}  // namespace firestore
}  // namespace firebase

#endif  // FIREBASE_FIRESTORE_CLIENT_CPP_SRC_ANDROID_CPP_POINTER_ANDROID_H_
