#include <node_api.h>
#include <immintrin.h>
#include <stdint.h>
#include <stddef.h>

#if defined(_MSC_VER)
  #define ZL_RESTRICT __restrict
  #define ZL_FORCEINLINE __forceinline
  #define ZL_ALIGN32 __declspec(align(32))
#else
  #define ZL_RESTRICT __restrict__
  #define ZL_FORCEINLINE inline __attribute__((always_inline))
  #define ZL_ALIGN32 __attribute__((aligned(32)))
#endif

// -----------------------------------------------------------------------------
// 1. Static Aligned SIMD State & Pinned Pointers
// -----------------------------------------------------------------------------
static ZL_ALIGN32 uint64_t g_barrier_lanes[4] = {
    0x04a1f8e9c201b7a6ULL, 0ULL, 0ULL, 0ULL
};
static __m256i g_expected_barrier_vec = _mm256_set_epi64x(
    0ULL, 0ULL, 0ULL, static_cast<long long>(0x04a1f8e9c201b7a6ULL)
);

// Pinned backing store pointers for zero-argument hot path
static const __m256i* g_pinned_tensor_ptr = nullptr;
static const float*   g_pinned_in_ptr     = nullptr;
static float*         g_pinned_out_ptr    = nullptr;
static size_t         g_pinned_len        = 256;

// One-time setup: lattice.setBarrierToken(token)
static napi_value SetBarrierToken(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value arg0;
    napi_get_cb_info(env, info, &argc, &arg0, nullptr, nullptr);

    if (argc >= 1) {
        uint64_t token = 0;
        bool lossless = false;
        napi_get_value_bigint_uint64(env, arg0, &token, &lossless);
        g_barrier_lanes[0] = token;
        g_expected_barrier_vec = _mm256_load_si256(
            reinterpret_cast<const __m256i*>(g_barrier_lanes)
        );
        _mm_mfence();
    }

    napi_value result;
    napi_get_boolean(env, true, &result);
    return result;
}

// One-time startup binding: lattice.bindBuffers(tensor, input, output)
static napi_value BindBuffers(napi_env env, napi_callback_info info) {
    size_t argc = 3;
    napi_value args[3];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    void* t_ptr = nullptr;
    void* in_ptr = nullptr;
    void* out_ptr = nullptr;
    size_t in_len = 0;

    if (argc >= 1) {
        napi_get_typedarray_info(env, args[0], nullptr, nullptr, &t_ptr, nullptr, nullptr);
        g_pinned_tensor_ptr = static_cast<const __m256i*>(t_ptr);
    }
    if (argc >= 2) {
        napi_get_typedarray_info(env, args[1], nullptr, &in_len, &in_ptr, nullptr, nullptr);
        g_pinned_in_ptr = static_cast<const float*>(in_ptr);
        g_pinned_len = in_len;
    }
    if (argc >= 3) {
        napi_get_typedarray_info(env, args[2], nullptr, nullptr, &out_ptr, nullptr, nullptr);
        g_pinned_out_ptr = static_cast<float*>(out_ptr);
    }

    napi_value res;
    napi_get_boolean(env, true, &res);
    return res;
}

// -----------------------------------------------------------------------------
// 2. Zero-Argument Pinned Fast Path (~15–22 ns)
// -----------------------------------------------------------------------------
static napi_value VerifyBarrierPinned(napi_env env, napi_callback_info /*info*/) {
    const __m256i vec_tensor = _mm256_loadu_si256(g_pinned_tensor_ptr);
    const __m256i cmp_mask   = _mm256_cmpeq_epi64(vec_tensor, g_expected_barrier_vec);
    const int bitmask        = _mm256_movemask_epi8(cmp_mask);
    _mm_lfence();

    napi_value result;
    napi_get_boolean(env, (bitmask == -1), &result);
    return result;
}

// -----------------------------------------------------------------------------
// 3. Hot-Path SIMD Manifold Projection Kernel (AVX2 / FMA3)
// -----------------------------------------------------------------------------
static ZL_FORCEINLINE void simd_project_manifold_256(
    const float* ZL_RESTRICT in_ptr,
    float* ZL_RESTRICT out_ptr,
    size_t len) noexcept {

#if defined(__AVX512F__)
    const __m512 scale = _mm512_set1_ps(0.998046875f);
    const __m512 bias  = _mm512_set1_ps(0.001953125f);
    size_t i = 0;
    for (; i + 64 <= len; i += 64) {
        __m512 v0 = _mm512_loadu_ps(in_ptr + i);
        __m512 v1 = _mm512_loadu_ps(in_ptr + i + 16);
        __m512 v2 = _mm512_loadu_ps(in_ptr + i + 32);
        __m512 v3 = _mm512_loadu_ps(in_ptr + i + 48);

        _mm512_storeu_ps(out_ptr + i,      _mm512_fmadd_ps(v0, scale, bias));
        _mm512_storeu_ps(out_ptr + i + 16, _mm512_fmadd_ps(v1, scale, bias));
        _mm512_storeu_ps(out_ptr + i + 32, _mm512_fmadd_ps(v2, scale, bias));
        _mm512_storeu_ps(out_ptr + i + 48, _mm512_fmadd_ps(v3, scale, bias));
    }
    for (; i < len; ++i) {
        out_ptr[i] = in_ptr[i] * 0.998046875f + 0.001953125f;
    }
#else
    const __m256 scale = _mm256_set1_ps(0.998046875f);
    const __m256 bias  = _mm256_set1_ps(0.001953125f);

    size_t i = 0;
    for (; i + 32 <= len; i += 32) {
        __m256 y0 = _mm256_loadu_ps(in_ptr + i);
        __m256 y1 = _mm256_loadu_ps(in_ptr + i + 8);
        __m256 y2 = _mm256_loadu_ps(in_ptr + i + 16);
        __m256 y3 = _mm256_loadu_ps(in_ptr + i + 24);

        y0 = _mm256_fmadd_ps(y0, scale, bias);
        y1 = _mm256_fmadd_ps(y1, scale, bias);
        y2 = _mm256_fmadd_ps(y2, scale, bias);
        y3 = _mm256_fmadd_ps(y3, scale, bias);

        _mm256_storeu_ps(out_ptr + i,      y0);
        _mm256_storeu_ps(out_ptr + i + 8,  y1);
        _mm256_storeu_ps(out_ptr + i + 16, y2);
        _mm256_storeu_ps(out_ptr + i + 24, y3);
    }
    for (; i + 8 <= len; i += 8) {
        __m256 y = _mm256_loadu_ps(in_ptr + i);
        _mm256_storeu_ps(out_ptr + i, _mm256_fmadd_ps(y, scale, bias));
    }
    for (; i < len; ++i) {
        out_ptr[i] = in_ptr[i] * 0.998046875f + 0.001953125f;
    }
#endif
}

static napi_value ProjectVectorPinned(napi_env env, napi_callback_info /*info*/) {
    simd_project_manifold_256(g_pinned_in_ptr, g_pinned_out_ptr, g_pinned_len);
    napi_value result;
    napi_get_boolean(env, true, &result);
    return result;
}

// Backward-compatible dynamic methods
static napi_value VerifyBarrier(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    void* tensor_data = nullptr;
    napi_get_typedarray_info(env, args[0], nullptr, nullptr, &tensor_data, nullptr, nullptr);

    const __m256i vec_tensor = _mm256_loadu_si256(static_cast<const __m256i*>(tensor_data));
    __m256i vec_target = g_expected_barrier_vec;

    if (argc >= 2) {
        uint64_t override_token = 0;
        bool lossless = false;
        napi_get_value_bigint_uint64(env, args[1], &override_token, &lossless);
        vec_target = _mm256_set_epi64x(0ULL, 0ULL, 0ULL, static_cast<long long>(override_token));
    }

    const __m256i cmp_mask = _mm256_cmpeq_epi64(vec_tensor, vec_target);
    const int bitmask      = _mm256_movemask_epi8(cmp_mask);
    _mm_lfence();

    napi_value result;
    napi_get_boolean(env, (bitmask == -1), &result);
    return result;
}

static napi_value ProjectVector(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    void* in_data = nullptr;
    size_t in_len = 0;
    napi_get_typedarray_info(env, args[0], nullptr, &in_len, &in_data, nullptr, nullptr);
    const float* in_ptr = static_cast<const float*>(in_data);

    if (argc >= 2) {
        void* out_data = nullptr;
        size_t out_len = 0;
        napi_get_typedarray_info(env, args[1], nullptr, &out_len, &out_data, nullptr, nullptr);
        float* out_ptr = static_cast<float*>(out_data);
        simd_project_manifold_256(in_ptr, out_ptr, (in_len < out_len) ? in_len : out_len);
        return args[1];
    }

    simd_project_manifold_256(in_ptr, static_cast<float*>(in_data), in_len);
    return args[0];
}

// -----------------------------------------------------------------------------
// 4. Module Initialization
// -----------------------------------------------------------------------------
static napi_value Init(napi_env env, napi_value exports) {
    napi_property_descriptor desc[] = {
        { "setBarrierToken",      nullptr, SetBarrierToken,      nullptr, nullptr, nullptr, napi_default, nullptr },
        { "bindBuffers",          nullptr, BindBuffers,          nullptr, nullptr, nullptr, napi_default, nullptr },
        { "verifyBarrierPinned",  nullptr, VerifyBarrierPinned,  nullptr, nullptr, nullptr, napi_default, nullptr },
        { "projectVectorPinned",  nullptr, ProjectVectorPinned,  nullptr, nullptr, nullptr, napi_default, nullptr },
        { "verifyBarrier",        nullptr, VerifyBarrier,        nullptr, nullptr, nullptr, napi_default, nullptr },
        { "projectVector",        nullptr, ProjectVector,        nullptr, nullptr, nullptr, napi_default, nullptr }
    };
    napi_define_properties(env, exports, 6, desc);
    return exports;
}

NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)