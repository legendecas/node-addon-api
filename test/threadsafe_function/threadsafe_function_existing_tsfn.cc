#include <cstdlib>
#include "napi.h"
#include "test_helper.h"

#if (NAPI_VERSION > 3)

using namespace Napi;

namespace {

struct TestContext {
  TestContext(Promise::Deferred &&deferred)
      : deferred(std::move(deferred)), callData(nullptr){};

  napi_threadsafe_function tsfn;
  Promise::Deferred deferred;
  double *callData;

  ~TestContext() {
    if (callData != nullptr)
      delete callData;
  };
};

void FinalizeCB(napi_env env, void * /*finalizeData */, void *context) {
  TestContext *testContext = static_cast<TestContext *>(context);
  if (testContext->callData != nullptr) {
    testContext->deferred.Resolve(Number::New(env, *testContext->callData));
  } else {
    testContext->deferred.Resolve(Napi::Env(env).Undefined());
  }
  delete testContext;
}

void CallJSWithData(napi_env env, napi_value /* callback */, void *context,
                    void *data) {
  TestContext *testContext = static_cast<TestContext *>(context);
  testContext->callData = static_cast<double *>(data);

  napi_status status =
      napi_release_threadsafe_function(testContext->tsfn, napi_tsfn_release);

  NAPI_THROW_IF_FAILED_VOID(env, status);
}

void CallJSNoData(napi_env env, napi_value /* callback */, void *context,
                  void * /*data*/) {
  TestContext *testContext = static_cast<TestContext *>(context);
  testContext->callData = nullptr;

  napi_status status =
      napi_release_threadsafe_function(testContext->tsfn, napi_tsfn_release);

  NAPI_THROW_IF_FAILED_VOID(env, status);
}

static Value TestCall(const CallbackInfo &info) {
  Napi::Env env = info.Env();
  bool isBlocking = false;
  bool hasData = false;
  if (info.Length() > 0) {
    Object opts = info[0].As<Object>();
    if (opts.Has("blocking")) {
      isBlocking =
          MaybeToChecked(MaybeToChecked(opts.Get("blocking")).ToBoolean());
    }
    if (opts.Has("data")) {
      hasData = MaybeToChecked(MaybeToChecked(opts.Get("data")).ToBoolean());
    }
  }

  // Allow optional callback passed from JS. Useful for testing.
  Function cb = Function::New(env, [](const CallbackInfo & /*info*/) {});

  TestContext *testContext = new TestContext(Napi::Promise::Deferred(env));

  napi_status status = napi_create_threadsafe_function(
      env, cb, Object::New(env), String::New(env, "Test"), 0, 1,
      nullptr, /*finalize data*/
      FinalizeCB, testContext, hasData ? CallJSWithData : CallJSNoData,
      &testContext->tsfn);

  NAPI_THROW_IF_FAILED(env, status, Value());

  ThreadSafeFunction wrapped = ThreadSafeFunction(testContext->tsfn);

  // Test the four napi_threadsafe_function direct-accessing calls
  if (isBlocking) {
    if (hasData) {
      wrapped.BlockingCall(static_cast<void *>(new double(std::rand())));
    } else {
      wrapped.BlockingCall(static_cast<void *>(nullptr));
    }
  } else {
    if (hasData) {
      wrapped.NonBlockingCall(static_cast<void *>(new double(std::rand())));
    } else {
      wrapped.NonBlockingCall(static_cast<void *>(nullptr));
    }
  }

  return testContext->deferred.Promise();
}

} // namespace

Object InitThreadSafeFunctionExistingTsfn(Env env) {
  Object exports = Object::New(env);
  exports["testCall"] = Function::New(env, TestCall);

  return exports;
}

#endif
