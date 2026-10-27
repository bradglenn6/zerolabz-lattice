#include <iostream>
#include <zerolabz/lattice.hpp>

int main() {
    using namespace zerolabz;
    StatelessSubMicroMapper<256, 16> mapper;

    alignas(64) float input[256];
    alignas(64) float output[16] = {0.0f};
    for (size_t i = 0; i < 256; ++i) input[i] = 1.0f;

    // Warm-up cache
    mapper.project<Manifold::OrthogonalIsometry>(input, output);

    uint64_t start = read_tsc();
    mapper.project<Manifold::OrthogonalIsometry>(input, output);
    uint64_t elapsed_cycles = read_tsc() - start;

    std::cout << "[ZeroLabz] Sub-Microsecond Kernel Execution: " 
              << elapsed_cycles << " CPU Cycles\n";
    return 0;
}
