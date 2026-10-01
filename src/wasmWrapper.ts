// Wrapper to initialize and call compiled Wasm persistence loops
export class BraidMeshTdaWasmAdapter {
  private wasmModule: any;

  constructor(wasmModuleInstance: any) {
    this.wasmModule = wasmModuleInstance;
  }

  public inspectEmbeddings(vectors: number[][], epsilon: number) {
    const n = vectors.length;
    if (n === 0) return null;
    const dim = vectors[0].length;

    // Flatten 2D JS Array to 1D Float Vector for native C++ memory access
    const flatVector = new this.wasmModule.VectorDouble();
    for (let i = 0; i < n; i++) {
      for (let d = 0; d < dim; d++) {
        flatVector.push_back(vectors[i][d]);
      }
    }

    // Call native C++ module via Emscripten bindings
    const result = this.wasmModule.TdaEngine.inspectEmbeddingSpaceWasm(
      flatVector,
      n,
      dim,
      epsilon
    );

    // Free native memory buffer
    flatVector.delete();

    return result;
  }
}
