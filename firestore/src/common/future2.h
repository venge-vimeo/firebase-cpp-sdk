/*
 * Copyright 2021 Google LLC
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

#ifndef FIREBASE_FIRESTORE_SRC_COMMON_FUTURE2_H_
#define FIREBASE_FIRESTORE_SRC_COMMON_FUTURE2_H_

#include <functional>
#include <string>

namespace firebase {

enum Future2Status {
  kFutureStatusComplete,
  kFutureStatusPending,
  kFutureStatusInvalid
};

class Future2CompleterBase;

class Future2Base {
 public:
  Future2Base();
  ~Future2Base();

  Future2Base(const Future2Base&);
  Future2Base& operator=(const Future2Base&);

  Future2Base(Future2Base&&) noexcept;
  Future2Base& operator=(Future2Base&&) noexcept;

  Future2Status status() const;
  int error() const;
  std::string error_message() const;
  const void* result_void() const;

  typedef void (*CompletionCallback)(const Future2Base&, void*);
  void OnCompletion(CompletionCallback, void*) const;
  void OnCompletion(std::function<void(const Future2Base&)>) const;

 private:
  friend class Future2CompleterBase;
  class Impl;
  Impl* impl_ = nullptr;
};

template <typename T>
class Future2 : public Future2Base {
 public:
  using Future2Base::Future2Base;

  const T* result() const {
    return static_cast<const T*>(result_void());
  }

  typedef void (*TypedCompletionCallback)(const Future2&, void*);

  void OnCompletion(TypedCompletionCallback callback, void* user_data) const {
    Future2Base::OnCompletion(reinterpret_cast<CompletionCallback>(callback), user_data);
  }

  void OnCompletion(std::function<void(const Future2&)> callback) const {
    Future2Base::OnCompletion(*reinterpret_cast<std::function<void(const Future2Base&)>*>(&callback));
  }
};

}  // namespace firebase

#endif  // FIREBASE_FIRESTORE_SRC_COMMON_FUTURE2_H_
