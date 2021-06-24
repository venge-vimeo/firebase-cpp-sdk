#include "firestore/src/android/cpp_pointer_android.h"

#include <cstddef>

#include "firestore/src/jni/declaration.h"
#include "firestore/src/jni/loader.h"

namespace firebase {
namespace firestore {

using jni::Constructor;
using jni::Loader;
using jni::Method;

namespace {

constexpr char kClass[] = PROGUARD_KEEP_CLASS "com/google/firebase/firestore/internal/cpp/CppPointer";

Constructor<JavaCppPointerBase> kConstructor("(J)V");
Method<int64_t> kGetPointer("getPointer", "()J");

}  // namespace

void JavaCppPointerBase::Initialize(Loader& loader) {
  loader.LoadClass(kClass, kConstructor, kGetPointer);
}

}  // namespace firestore
}  // namespace firebase
