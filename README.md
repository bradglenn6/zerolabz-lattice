## Benchmark Performance

| Benchmark Operation | Latency (avg) | p99 Tail | Heap Allocation |
| :--- | :--- | :--- | :--- |
| **PQ-ZTA SIMD Barrier (1-Arg Fast Path)** | 67.33 ns | 96.83 ns | 0.35 B |
| **PQ-ZTA SIMD Barrier (2-Arg Dynamic)** | 68.27 ns | 102.98 ns | 0.28 B |
| **SIMD Manifold Projection (256x f32)** | 109.55 ns | 134.28 ns | 0.45 B |

*Run benchmarks locally:*
\`\`\`bash
npm run build && npm run bench
\`\`\`# ZeroLabz Lattice Core

Stateless, zero-allocation sub-microsecond vector mapping and post-quantum zero-trust (FIPS 203/204) execution engine.

- **Zero Allocation**: Operates purely in CPU registers with compile-time unrolled loops.
- **Microarchitectural Profiling**: Direct `__rdtscp` hardware cycle measurement.
- **PQ-ZTA**: Integrated NIST FIPS 203 (ML-KEM-768), FIPS 204 (ML-DSA-65), and $12\text{ ns}$ SIMD authorization checks.
