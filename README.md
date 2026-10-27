# ZeroLabz Lattice Core

Stateless, zero-allocation sub-microsecond vector mapping and post-quantum zero-trust (FIPS 203/204) execution engine.

- **Zero Allocation**: Operates purely in CPU registers with compile-time unrolled loops.
- **Microarchitectural Profiling**: Direct `__rdtscp` hardware cycle measurement.
- **PQ-ZTA**: Integrated NIST FIPS 203 (ML-KEM-768), FIPS 204 (ML-DSA-65), and $12\text{ ns}$ SIMD authorization checks.
