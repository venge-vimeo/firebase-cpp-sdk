#include "util/future_test_util.h"

#include <ostream>

#include "firebase/firestore/firestore_errors.h"
#include "firestore_integration_test.h"

using firebase_test_framework::FirebaseTest;
using firebase::Future;
using firebase::FutureStatus;
using firebase::firestore::Error;

namespace {

void PrintToImpl(std::ostream* os, FutureStatus future_status, int error, const char* error_message = nullptr) {
  *os << "Future<void>{status=" << firebase_testapp_automated::ToEnumeratorName(future_status) << " error=" << firebase_testapp_automated::ToFirestoreErrorCodeName(error);
  if (error_message != nullptr) {
    *os << " error_message=" << error_message;
  }
  *os << "}";
}

class FutureSucceedsImpl : public testing::MatcherInterface<const Future<void>&> {
 public:
  void DescribeTo(std::ostream* os) const override {
    PrintToImpl(os, FutureStatus::kFutureStatusComplete, Error::kErrorOk);
  }

  bool MatchAndExplain(const Future<void>& future, testing::MatchResultListener*) const override {
    FirebaseTest::WaitForCompletionAnyResult(future, "FutureSucceeds");
    return future.status() == FutureStatus::kFutureStatusComplete && future.error() == Error::kErrorOk;
  }
};

}  // namespace

namespace firebase_testapp_automated {

std::string ToEnumeratorName(FutureStatus status) {
  switch (status) {
    case FutureStatus::kFutureStatusComplete:
      return "kFutureStatusComplete";
    case FutureStatus::kFutureStatusPending:
      return "kFutureStatusPending";
    case FutureStatus::kFutureStatusInvalid:
      return "kFutureStatusInvalid";
    default:
      return "[invalid FutureStatus]";
  }
}

testing::Matcher<const Future<void>&> FutureSucceeds() {
  return testing::Matcher<const Future<void>&>(new FutureSucceedsImpl());
}

}  // namespace firebase_testapp_automated

namespace firebase {

void PrintTo(const Future<void>& future, std::ostream* os) {
  PrintToImpl(os, future.status(), future.error(), future.error_message());
}

}  // namespace firebase
