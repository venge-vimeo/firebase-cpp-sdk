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

#include "firestore/src/common/future2.h"
#include "firestore/src/common/future2_internal.h"

#include "app/memory/shared_ptr.h"
#include "app/src/include/firebase/internal/mutex.h"

using ::firebase::MakeShared;
using ::firebase::Mutex;
using ::firebase::MutexLock;
using ::firebase::SharedPtr;

namespace firebase {

namespace {

class Future2ControlBlock {
 public:
  Future2ControlBlock() : mutex_(Mutex::Mode::kModeNonRecursive) {
  }

  Future2ControlBlock(const Future2ControlBlock&) = delete;
  Future2ControlBlock& operator=(const Future2ControlBlock&) = delete;
  Future2ControlBlock(Future2ControlBlock&&) = delete;
  Future2ControlBlock& operator=(Future2ControlBlock&&) = delete;

  Future2Status status() const {
    MutexLock lock(mutex_);
    return status_;
  }

  int error() const {
    MutexLock lock(mutex_);
    return error_;
  }

  const char* error_message() const {
    MutexLock lock(mutex_);
    return error_message_;
  }

  const void* result() const {
    MutexLock lock(mutex_);
    return result_;
  }
 
 private:
  mutable Mutex mutex_;
  Future2Status status_ = Future2Status::kFutureStatusInvalid;
  int error_ = -1;
  const char* error_message_ = nullptr;
  const void* result_ = nullptr;
};

}  // namespace

class Future2Base::Impl {
 public:
  Impl() : control_block_(MakeShared<Future2ControlBlock>()) {
  }

  Future2ControlBlock& control_block() {
    return *control_block_;
  }

 private:
  SharedPtr<Future2ControlBlock> control_block_;
};

Future2Base::Future2Base() : impl_(new Impl()) {
}

Future2Base::~Future2Base() {
  delete impl_;
}

Future2Base::Future2Base(const Future2Base& other) : impl_(other.impl_ ? new Impl(*other.impl_) : nullptr) {
}

Future2Base::Future2Base(Future2Base&& other) noexcept : impl_(other.impl_) {
  other.impl_ = nullptr;
}

Future2Base& Future2Base::operator=(const Future2Base& other) {
  if (&other != this) {
    Impl* old_impl = impl_;
    impl_ = nullptr;
    delete old_impl;

    if (other.impl_) {
      impl_ = new Impl(*other.impl_);
    }
  }
  return *this;
}

Future2Base& Future2Base::operator=(Future2Base&& other) noexcept {
  if (&other != this) {
    Impl* old_impl = impl_;
    impl_ = nullptr;
    delete old_impl;

    impl_ = other.impl_;
    other.impl_ = nullptr;
  }
  return *this;
}

Future2Status Future2Base::status() const {
  return impl_->control_block().status();
}

int Future2Base::error() const {
  return impl_->control_block().error();
}

const char* Future2Base::error_message() const {
  return impl_->control_block().error_message();
}

const void* Future2Base::result_void() const {
  return impl_->control_block().result();
}

}  // namespace firebase
