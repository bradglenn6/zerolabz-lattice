import { createRequire } from 'module';
const require = createRequire(import.meta.url);
const lattice = require('../build/Release/lattice_native.node');

const input = new Float32Array(256);
input.fill(1.0);

// Warmup JIT & L1 Cache
for (let i = 0; i < 1000; ++i) {
    lattice.projectVector(input);
}

// Benchmark 10,000 iterations
const iterations = 10000;
const start = performance.now();
for (let i = 0; i < iterations; ++i) {
    lattice.projectVector(input);
}
const elapsedMs = performance.now() - start;
const avgUs = (elapsedMs * 1000) / iterations;

console.log(`[ZeroLabz] Steady-State Average Latency: ${avgUs.toFixed(3)} µs / op`);
console.log(`[ZeroLabz] Native Throughput: ${(iterations / (elapsedMs / 1000)).toLocaleString()} ops / sec`);

// SIMD Barrier Verification
const tensor = new BigInt64Array([0x04a1f8e9c201b7a6n, 0x0n, 0x0n, 0x0n]);
const authorized = lattice.verifyBarrier(tensor, 0x04a1f8e9c201b7a6n);
console.log("[ZeroLabz] PQ-ZTA SIMD Barrier Auth:", authorized ? "VERIFIED (Pass)" : "REJECTED (Fail)");