#include "firestore_integration_test.h"

#include "app_framework.h"
#include "firebase/auth.h"
#include "util/autoid.h"
#include "util/future_test_util.h"

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

using ::firebase::firestore::Error;
using ::firebase::InitResult;
using ::firebase::ModuleInitializer;

namespace firebase_testapp_automated {

std::string ToFirestoreErrorCodeName(int error_code) {
  switch (error_code) {
    case Error::kErrorOk:
      return "kErrorOk";
    case Error::kErrorCancelled:
      return "kErrorCancelled";
    case Error::kErrorUnknown:
      return "kErrorUnknown";
    case Error::kErrorInvalidArgument:
      return "kErrorInvalidArgument";
    case Error::kErrorDeadlineExceeded:
      return "kErrorDeadlineExceeded";
    case Error::kErrorNotFound:
      return "kErrorNotFound";
    case Error::kErrorAlreadyExists:
      return "kErrorAlreadyExists";
    case Error::kErrorPermissionDenied:
      return "kErrorPermissionDenied";
    case Error::kErrorResourceExhausted:
      return "kErrorResourceExhausted";
    case Error::kErrorFailedPrecondition:
      return "kErrorFailedPrecondition";
    case Error::kErrorAborted:
      return "kErrorAborted";
    case Error::kErrorOutOfRange:
      return "kErrorOutOfRange";
    case Error::kErrorUnimplemented:
      return "kErrorUnimplemented";
    case Error::kErrorInternal:
      return "kErrorInternal";
    case Error::kErrorUnavailable:
      return "kErrorUnavailable";
    case Error::kErrorDataLoss:
      return "kErrorDataLoss";
    case Error::kErrorUnauthenticated:
      return "kErrorUnauthenticated";
    default:
      return "[invalid error code]";
  }
}

void FirestoreIntegrationTest::SetUpTestSuite() {
  // Look for google-services.json and change the current working directory to
  // the directory that contains it, if found.
  FindFirebaseConfig(FIREBASE_CONFIG_STRING);
}

void FirestoreIntegrationTest::TearDown() {
  if (firestore_) {
    EXPECT_THAT(firestore_->Terminate(), FutureSucceeds());
    EXPECT_THAT(firestore_->ClearPersistence(), FutureSucceeds());
  }

  delete firestore_;
  delete auth_;
  delete app_;
}

App* FirestoreIntegrationTest::app() {
  if (app_ != nullptr) {
    return app_;
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

  app_ = app;
  return app;
}

Firestore* FirestoreIntegrationTest::TestFirestore(const std::string& app_name) {
  const std::string scoped_trace_name = std::string("TestFirestore(") + app_name + ")";
  SCOPED_TRACE(scoped_trace_name.c_str());

  if (app_name != kDefaultAppName) {
    // TODO(support non-default app_name)
    ADD_FAILURE() << std::string("non-default app name not supported yet: ") + app_name;
    abort();
  } else if (default_firestore_created_) {
    // TODO(support caching the Firestore instance)
    ADD_FAILURE() << "TestFirestore() has already been invoked once";
    abort();
  }

  if (firestore_ != nullptr) {
    return firestore_;
  }

  App* app = nullptr;
  {
    SCOPED_TRACE("CreateApp");
    app = this->app();
  }

  if (auth_ == nullptr) {
    SCOPED_TRACE("InitializeAuth");
    ModuleInitializer initializer;
    auto initialize_auth_future = initializer.Initialize(app, &auth_, [](App* app, void* target) {
      InitResult result;
      *reinterpret_cast<Auth**>(target) = Auth::GetAuth(app, &result);
      return result;
    });
    WaitForCompletion(initialize_auth_future, "InitializeAuth");
    if (initialize_auth_future.error() != 0) {
      abort();
    }
  }

  if (auth_ == nullptr) {
    ADD_FAILURE() << "Auth::GetAuth() returned nullptr";
    abort();
  }

  if (auth_->current_user() == nullptr) {
    SCOPED_TRACE("SignInAnonymously");
    auto sign_in_future = auth_->SignInAnonymously();
    WaitForCompletion(sign_in_future, "SignInAnonymously");
    if (sign_in_future.error() != 0) {
      abort();
    }
  }

  if (firestore_ == nullptr) {
    SCOPED_TRACE("InitializeFirestore");
    ModuleInitializer initializer;
    auto initialize_firestore_future = initializer.Initialize(app, &firestore_, [](App* app, void* target) {
      InitResult result;
      *reinterpret_cast<Firestore**>(target) = Firestore::GetInstance(app, &result);
      return result;
    });
    WaitForCompletion(initialize_firestore_future, "InitializeFirestore");
    if (initialize_firestore_future.error() != 0) {
      abort();
    }
  }

  if (firestore_ == nullptr) {
    ADD_FAILURE() << "Firestore::GetInstance() returned nullptr";
    abort();
  }

  default_firestore_created_ = true;
  return firestore_;
}

CollectionReference FirestoreIntegrationTest::Collection(const std::string& name_prefix) {
  const std::string scoped_trace_name = std::string("Collection(") + name_prefix + ")";
  SCOPED_TRACE(scoped_trace_name.c_str());
  return TestFirestore()->Collection(name_prefix + "_" + CreateAutoIdForTesting());
}

DocumentSnapshot FirestoreIntegrationTest::ReadDocument(const DocumentReference& reference) {
  const std::string scoped_trace_name = std::string("ReadDocument(") + reference.ToString() + ")";
  SCOPED_TRACE(scoped_trace_name.c_str());
  Future<DocumentSnapshot> future = reference.Get();
  if (!WaitForCompletion(future, "ReadDocument")) {
    return {};
  }
  return *future.result();
}

}  // namespace firebase_testapp_automated
