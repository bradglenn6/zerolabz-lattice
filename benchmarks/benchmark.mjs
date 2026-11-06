import { run, bench, group, baseline } from 'mitata';
import crypto from 'node:crypto';
import { memoryGuard, fips203 } from '@zerolabz/lattice-core'; 

// 1088 bytes matches FIPS 203 ML-KEM-1024 ciphertext size
const BUFFER_SIZE = 1088;
const standardBuffer = Buffer.alloc(BUFFER_SIZE);

group('N-API Boundary: Memory Barriers', () => {
  baseline('V8 Standard Buffer.alloc', () => {
    Buffer.alloc(BUFFER_SIZE);
  });

  bench('Lattice mlock + MADV_DONTDUMP Allocation', () => {
    // Measures Rust FFI boundary crossing for secure page-locking
    const secureBuf = memoryGuard.allocProtected(BUFFER_SIZE);
    
    // Explicit free to avoid V8 GC jitter during the run
    memoryGuard.freeProtected(secureBuf); 
  });

  bench('AVX-512 Volatile Zeroization (12ns Target)', () => {
    // Tests passing an existing Node buffer down to the Rust SIMD routines
    memoryGuard.volatileZeroize(standardBuffer);
  });
});

group('FIPS 203 ML-KEM-1024 Latency', () => {
  baseline('Node Native crypto.randomFillSync', () => {
    crypto.randomFillSync(standardBuffer);
  });

  bench('Lattice Core: ML-KEM-1024 Encapsulation', () => {
    // Isolates the N-API type conversion overhead against the lattice math
    fips203.encapsulate();
  });
});

// Execute the suite with high-resolution telemetry
await run({
  avg: true,
  colors: true,
  min_max: true,
  percentiles: true // Crucial for catching p99 GC latency spikes
});