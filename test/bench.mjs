import { run, bench, group } from 'mitata';
import { createRequire } from 'module';

const require = createRequire(import.meta.url);
const lattice = require('../build/Release/lattice_native.node');

// 1. One-Time Static Barrier Registration & Verification
const TOKEN = 0x04a1f8e9c201b7a6n;
const tensor = new BigInt64Array([TOKEN, 0x0n, 0x0n, 0x0n]);

if (typeof lattice.setBarrierToken === 'function') {
  lattice.setBarrierToken(TOKEN);
}

const authorized = lattice.verifyBarrier(tensor);
console.log(`\n[ZeroLabz] PQ-ZTA SIMD Barrier Auth: ${authorized ? "VERIFIED (Pass)" : "REJECTED (Fail)"}\n`);

if (!authorized) {
  process.exit(1);
}

// 2. Pre-Allocated Zero-Copy Input & Output Buffers
const input = new Float32Array(256);
const output = new Float32Array(256);
input.fill(1.0);

let sink = null;

// 3. High-Resolution Benchmarks
group('ZeroLabz Lattice Core N-API Latency', () => {
  bench('PQ-ZTA SIMD Barrier (Pre-Registered 1-Arg Fast Path)', () => {
    sink = lattice.verifyBarrier(tensor);
  }).baseline();

  bench('PQ-ZTA SIMD Barrier (Dynamic 2-Arg BigInt Unboxing)', () => {
    sink = lattice.verifyBarrier(tensor, TOKEN);
  });

  bench('AVX-512 / AVX2 SIMD Manifold Projection (Zero-Copy)', () => {
    sink = lattice.projectVector(input, output);
  });
});

// 4. Execute Suite
await run({
  avg: true,
  colors: true,
  min_max: true,
  percentiles: true
});

if (sink === undefined) process.exit(1);
