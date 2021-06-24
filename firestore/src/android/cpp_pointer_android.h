#ifndef FIREBASE_FIRESTORE_CLIENT_CPP_SRC_ANDROID_CPP_POINTER_ANDROID_H_
#define FIREBASE_FIRESTORE_CLIENT_CPP_SRC_ANDROID_CPP_POINTER_ANDROID_H_

#include <cstddef>

#include "firestore/src/jni/jni_fwd.h"
#include "firestore/src/jni/object.h"

namespace firebase {
namespace firestore {

class JavaCppPointerBase : public jni::Object {
 public:
  using Object::Object;

  static void Initialize(jni::Loader& loader);

 protected:
  void* GetPointer(jni::Env& env) const;
};

template <typename T>
class JavaCppPointer : JavaCppPointerBase {
 public:
  using JavaCppPointerBase::JavaCppPointerBase;

  T* GetPointer(jni::Env& env) const;
};

}  // namespace firestore
}  // namespace firebase

#endif  // FIREBASE_FIRESTORE_CLIENT_CPP_SRC_ANDROID_CPP_POINTER_ANDROID_H_
