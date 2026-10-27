const path = require('node:path');
const native = require(path.join(__dirname, '../build/Release/lattice_native.node'));

export interface ILatticeCore {
  projectVector(input: Float32Array): Float32Array;
  verifyBarrier(tokenTensor: BigInt64Array, expectedHash: bigint): boolean;
}

export const projectVector = (input: Float32Array): Float32Array => {
  return native.projectVector(input);
};

export const verifyBarrier = (tokenTensor: BigInt64Array, expectedHash: bigint): boolean => {
  return native.verifyBarrier(tokenTensor, expectedHash);
};

export default {
  projectVector,
  verifyBarrier
};