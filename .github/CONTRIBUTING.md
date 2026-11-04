# Contributing to ZeroLabz Lattice

Thank you for contributing to the `@zerolabz/lattice-core` package and `zerolabz-lattice-coder` VS Code extension.

## Development Workflow

1. **Fork the repository** and create a feature branch (`git checkout -b feature/simd-optimization`).
2. **Adhere to Standards**:
   - Ensure cryptographic implementations strictly follow FIPS 203 (ML-KEM) and FIPS 204 (ML-DSA) guidelines.
   - Run all memory barrier and SIMD benchmarking tests before submitting.
3. **Commit Messages**: Use concise conventional commit formats (`feat:`, `fix:`, `docs:`, `perf:`).
4. **Submitting Changes**:
   - Open a pull request against the `master` branch.
   - Include performance metrics if modifying AVX-512 routines or memory barrier latency.
