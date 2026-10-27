export interface ILatticeCore {
    projectVector(input: Float32Array): Float32Array;
    verifyBarrier(tokenTensor: BigInt64Array, expectedHash: bigint): boolean;
}
export declare const projectVector: (input: Float32Array) => Float32Array;
export declare const verifyBarrier: (tokenTensor: BigInt64Array, expectedHash: bigint) => boolean;
declare const _default: {
    projectVector: (input: Float32Array) => Float32Array;
    verifyBarrier: (tokenTensor: BigInt64Array, expectedHash: bigint) => boolean;
};
export default _default;
