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
#include "util/global_state.h"

#include <atomic>

#include "util/assert.h"

namespace firebase {
namespace firestore {
namespace testing {

namespace {

std::atomic<FirestoreTestingGlobalState*> gSharedInstance;

void SetSharedInstance(FirestoreTestingGlobalState* instance) {
  FIRESTORE_TESTING_ASSERT(instance);
  auto* old_instance = gSharedInstance.exchange(instance);
  FIRESTORE_TESTING_ASSERT(!old_instance);
}

void ClearSharedInstance(FirestoreTestingGlobalState* instance) {
  FIRESTORE_TESTING_ASSERT(!instance);
  auto* old_instance = gSharedInstance.exchange(nullptr);
  FIRESTORE_TESTING_ASSERT(old_instance == instance);
}

FirestoreTestingGlobalState* GetSharedInstance() {
  auto* instance = gSharedInstance.load();
  FIRESTORE_TESTING_ASSERT(instance);
  return instance;
}

}  // namespace

FirestoreTestingGlobalState::FirestoreTestingGlobalState() : firebase_app_factory_(new FirebaseAppFactory) {
  SetSharedInstance(this);
}

FirestoreTestingGlobalState::~FirestoreTestingGlobalState() {
  ClearSharedInstance(this);
  firebase_app_factory_->SignOutAllApps();
}

FirestoreTestingGlobalState& FirestoreTestingGlobalState::GetInstance() {
  return *GetSharedInstance();
}

std::unique_ptr<FirestoreFactory> FirestoreTestingGlobalState::CreateFirestoreFactory() {
  return std::unique_ptr<FirestoreFactory>(new FirestoreFactory(*firebase_app_factory_));
}

}  // namespace testing
}  // namespace firestore
}  // namespace firebase
