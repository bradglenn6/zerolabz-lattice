"use strict";
Object.defineProperty(exports, "__esModule", { value: true });
exports.verifyBarrier = exports.projectVector = void 0;
const path = require('node:path');
const native = require(path.join(__dirname, '../build/Release/lattice_native.node'));
const projectVector = (input) => {
    return native.projectVector(input);
};
exports.projectVector = projectVector;
const verifyBarrier = (tokenTensor, expectedHash) => {
    return native.verifyBarrier(tokenTensor, expectedHash);
};
exports.verifyBarrier = verifyBarrier;
exports.default = {
    projectVector: exports.projectVector,
    verifyBarrier: exports.verifyBarrier
};
