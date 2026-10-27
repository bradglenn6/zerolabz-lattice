#include <napi.h>
#include "zerolabz/lattice.hpp"

Napi::Value ProjectVector(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 1 || !info[0].IsTypedArray()) {
        Napi::TypeError::New(env, "Expected a TypedArray of dimension 256").ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::TypedArray typedArray = info[0].As<Napi::TypedArray>();
    if (typedArray.TypedArrayType() != napi_float32_array || typedArray.ElementLength() != 256) {
        Napi::TypeError::New(env, "Expected Float32Array with exactly 256 elements").ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::Float32Array srcArray = info[0].As<Napi::Float32Array>();
    Napi::Float32Array dstArray = Napi::Float32Array::New(env, 16);

    zerolabz::StatelessSubMicroMapper<256, 16> mapper;
    mapper.project<zerolabz::Manifold::OrthogonalIsometry>(srcArray.Data(), dstArray.Data());

    return dstArray;
}

Napi::Value VerifyBarrier(const Napi::CallbackInfo& info) {
    Napi::Env env = info.Env();

    if (info.Length() < 2 || !info[0].IsTypedArray() || !info[1].IsBigInt()) {
        Napi::TypeError::New(env, "Expected BigInt64Array and BigInt target hash").ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::TypedArray typedArray = info[0].As<Napi::TypedArray>();
    if (typedArray.TypedArrayType() != napi_bigint64_array) {
        Napi::TypeError::New(env, "Expected BigInt64Array").ThrowAsJavaScriptException();
        return env.Null();
    }

    Napi::BigInt64Array tensor = info[0].As<Napi::BigInt64Array>();
    bool lossless = false;
    uint64_t targetHash = info[1].As<Napi::BigInt>().Uint64Value(&lossless);

    bool ok = zerolabz::verify_simd_barrier(reinterpret_cast<const uint64_t*>(tensor.Data()), targetHash);
    return Napi::Boolean::New(env, ok);
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    exports.Set(Napi::String::New(env, "projectVector"), Napi::Function::New(env, ProjectVector));
    exports.Set(Napi::String::New(env, "verifyBarrier"), Napi::Function::New(env, VerifyBarrier));
    return exports;
}

NODE_API_MODULE(lattice_native, Init)