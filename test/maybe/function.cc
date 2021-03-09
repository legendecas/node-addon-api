#include "napi.h"

using namespace Napi;

namespace {

void VoidCallback(const CallbackInfo& info) {
  Function fn = info[0].As<Function>();

  Maybe<Value> it = fn.Call({});

  it.Check();
}

}  // end anonymous namespace

Object Init(Env env, Object exports) {
  exports.Set("voidCallback", Function::New(env, VoidCallback));
  return exports;
}

NODE_API_MODULE(addon, Init)
