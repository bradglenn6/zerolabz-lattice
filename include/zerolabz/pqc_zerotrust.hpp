#pragma once

#include <cstdint>
#include <string_view>
#include <array>
#include <immintrin.h>

namespace zerolabz {

// NIST FIPS 203 / 204 Specifications
struct PqcMetadata {
    static constexpr size_t ML_KEM_768_CIPHERTEXT_BYTES = 1088;
    static constexpr size_t ML_KEM_768_PUBLIC_KEY_BYTES = 1184;
    static constexpr size_t ML_DSA_65_SIGNATURE_BYTES   = 3309;
    static constexpr size_t ML_DSA_65_PUBLIC_KEY_BYTES  = 1952;
};

// Continuous Zero Trust O(1) in-register SIMD memory barrier check
inline bool verify_simd_barrier(const uint64_t* token_tensor, uint64_t expected_hash) noexcept {
#if defined(__AVX2__)
    __m256i token_vec = _mm256_load_si256(reinterpret_cast<const __m256i*>(token_tensor));
    __m256i target_vec = _mm256_set1_epi64x(expected_hash);
    __m256i cmp = _mm256_cmpeq_epi64(token_vec, target_vec);
    int mask = _mm256_movemask_epi8(cmp);
    return (mask & 0xFF) == 0xFF;
#else
    return token_tensor[0] == expected_hash;
#endif
}

} // namespace zerolabz
