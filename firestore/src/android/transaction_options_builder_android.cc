/*
 * Copyright 2022 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "firestore/src/android/transaction_options_builder_android.h"

#include "firestore/src/jni/env.h"
#include "firestore/src/jni/loader.h"

namespace firebase {
namespace firestore {
namespace {

using jni::Constructor;
using jni::Env;
using jni::Local;
using jni::Method;
using jni::Object;
using jni::String;

constexpr char kTransactionOptionsBuilderClass[] = PROGUARD_KEEP_CLASS "com/google/firebase/firestore/TransactionOptions$Builder";
Constructor<TransactionOptionsBuilderInternal> kNewBuilder("()V");
Method<TransactionOptionsBuilderInternal> kSetMaxAttempts("setMaxAttempts", "(I)Lcom/google/firebase/firestore/TransactionOptions$Builder;");
Method<TransactionOptionsInternal> kBuild("build", "()Lcom/google/firebase/firestore/TransactionOptions;");

}  // namespace

void TransactionOptionsBuilderInternal::Initialize(jni::Loader& loader) {
  loader.LoadClass(kTransactionOptionsBuilderClass, kNewBuilder, kSetMaxAttempts, kBuild);
}

Local<TransactionOptionsBuilderInternal> TransactionOptionsBuilderInternal::Create(Env& env) {
  return env.New(kNewBuilder);
}

jni::Local<TransactionOptionsBuilderInternal> TransactionOptionsBuilderInternal::SetMaxAttempts(jni::Env& env, int32_t max_attempts) const {
  return env.Call(*this, kSetMaxAttempts, max_attempts);
}

jni::Local<TransactionOptionsInternal> TransactionOptionsBuilderInternal::Build(jni::Env& env) const {
  return env.Call(*this, kBuild);
}

}  // namespace firestore
}  // namespace firebase
