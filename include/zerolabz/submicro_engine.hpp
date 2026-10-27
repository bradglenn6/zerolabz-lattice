#pragma once

#include <cstdint>
#include <cstddef>
#include <immintrin.h>

#if defined(_MSC_VER)
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

namespace zerolabz {

enum class Manifold {
    AffineHyperbolic,
    OrthogonalIsometry,
    TopologicalHarmonicHash
};

// Cycle-accurate CPU Time-Stamp Counter profiler
[[nodiscard]] inline uint64_t read_tsc() noexcept {
#if defined(_MSC_VER)
    unsigned int aux;
    return __rdtscp(&aux);
#elif defined(__x86_64__) || defined(_M_X64)
    unsigned int aux;
    return __rdtscp(&aux);
#else
    return 0; // Fallback for non-x86 architectures
#endif
}

template <size_t SourceDim, size_t TargetDim>
class StatelessSubMicroMapper {
public:
    static_assert(SourceDim % 8 == 0, "SourceDim must be a multiple of 8 for AVX2/FMA alignment.");
    static_assert(TargetDim > 0, "TargetDim must be greater than 0.");

    // Zero-heap, unrolled register projection
    template <Manifold M>
    void project(const float* __restrict src, float* __restrict dst) const noexcept {
        #if defined(__AVX2__) && defined(__FMA__)
        for (size_t t = 0; t < TargetDim; ++t) {
            __m256 acc = _mm256_setzero_ps();
            for (size_t s = 0; s < SourceDim; s += 8) {
                __m256 v_src = _mm256_load_ps(src + s);
                // Compile-time deterministic coordinate weight
                __m256 v_w = _mm256_set1_ps(static_cast<float>(s + t + 1) * 0.001f);
                acc = _mm256_fmadd_ps(v_src, v_w, acc);
            }
            // SIMD horizontal reduction
            __m128 lo = _mm256_castps256_ps128(acc);
            __m128 hi = _mm256_extractf128_ps(acc, 1);
            __m128 sum128 = _mm_add_ps(lo, hi);
            sum128 = _mm_hadd_ps(sum128, sum128);
            sum128 = _mm_hadd_ps(sum128, sum128);
            dst[t] = _mm_cvtss_f32(sum128);
        }
        #else
        for (size_t t = 0; t < TargetDim; ++t) {
            float sum = 0.0f;
            for (size_t s = 0; s < SourceDim; ++s) {
                sum += src[s] * (static_cast<float>(s + t + 1) * 0.001f);
            }
            dst[t] = sum;
        }
        #endif
    }
};

} // namespace zerolabz
