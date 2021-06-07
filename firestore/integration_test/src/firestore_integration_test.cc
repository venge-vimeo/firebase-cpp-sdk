#include "firestore_integration_test.h"

#include "app_framework.h"
#include "firebase/auth.h"

// The TO_STRING macro is useful for command line defined strings as the quotes
// get stripped.
#define TO_STRING_EXPAND(X) #X
#define TO_STRING(X) TO_STRING_EXPAND(X)

// Path to the Firebase config file to load.
#ifdef FIREBASE_CONFIG
#define FIREBASE_CONFIG_STRING TO_STRING(FIREBASE_CONFIG)
#else
#define FIREBASE_CONFIG_STRING ""
#endif  // FIREBASE_CONFIG

using ::firebase::App;
using ::firebase::InitResult;
using ::firebase::ModuleInitializer;
using ::firebase::auth::Auth;
using ::firebase::firestore::Firestore;
using ::firebase::kDefaultAppName;

namespace firebase_testapp_automated {

void FirestoreIntegrationTest::SetUpTestSuite() {
  // Look for google-services.json and change the current working directory to
  // the directory that contains it, if found.
  FindFirebaseConfig(FIREBASE_CONFIG_STRING);
}

std::shared_ptr<Firestore> FirestoreIntegrationTest::TestFirestore(const std::string& app_name) {
  if (app_name != kDefaultAppName) {
    // TODO(support non-default app_name)
    ADD_FAILURE() << std::string("non-default app name not supported yet: ") + app_name;
    abort();
  } else if (default_firestore_created_) {
    // TODO(support caching the Firestore instance)
    ADD_FAILURE() << "TestFirestore() has already been invoked once";
    abort();
  }

  #if defined(__ANDROID__)
    App* app = App::Create(app_framework::GetJniEnv(), app_framework::GetActivity());
  #else
    App* app = App::Create();
  #endif  // defined(__ANDROID__)

  if (app == nullptr) {
    ADD_FAILURE() << "App::Create() failed";
    abort();
  }

  Auth* auth = nullptr;
  {
    SCOPED_TRACE("InitializeAuth");
    ModuleInitializer initializer;
    auto initialize_auth_future = initializer.Initialize(app, &auth, [](App* app, void* target) {
      InitResult result;
      *reinterpret_cast<Auth**>(target) = Auth::GetAuth(app, &result);
      return result;
    });
    WaitForCompletion(initialize_auth_future, "InitializeAuth");
    if (initialize_auth_future.error() != 0) {
      abort();
    }
  }

  if (auth == nullptr) {
    ADD_FAILURE() << "Auth::GetAuth() returned nullptr";
    abort();
  }

  if (auth->current_user() == nullptr) {
    SCOPED_TRACE("SignInAnonymously");
    auto sign_in_future = auth->SignInAnonymously();
    WaitForCompletion(sign_in_future, "SignInAnonymously");
    if (sign_in_future.error() != 0) {
      abort();
    }
  }

  Firestore* firestore = nullptr;
  {
    SCOPED_TRACE("InitializeFirestore");
    ModuleInitializer initializer;
    auto initialize_firestore_future = initializer.Initialize(app, &firestore, [](App* app, void* target) {
      InitResult result;
      *reinterpret_cast<Firestore**>(target) = Firestore::GetInstance(app, &result);
      return result;
    });
    WaitForCompletion(initialize_firestore_future, "InitializeFirestore");
    if (initialize_firestore_future.error() != 0) {
      abort();
    }
  }

  default_firestore_created_ = true;
  return std::shared_ptr<Firestore>(firestore);
}

}  // namespace firebase_testapp_automatedfirebase
