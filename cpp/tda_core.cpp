#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <emscripten/bind.h>

using namespace emscripten;

struct TdaResult {
    int vector_count;
    int dimensions;
    double epsilon_threshold;
    int betti0_clusters;
    int betti1_loops;
    bool has_vector_traps;
    double compute_time_ms;
};

// Disjoint Set Union (DSU) for fast H0 Component Tracking
class DSU {
    std::vector<int> parent;
public:
    DSU(int n) {
        parent.resize(n);
        std::iota(parent.begin(), parent.end(), 0);
    }
    int find(int i) {
        if (parent[i] == i)
            return i;
        return parent[i] = find(parent[i]);
    }
    bool unite(int i, int j) {
        int root_i = find(i);
        int root_j = find(j);
        if (root_i != root_j) {
            parent[root_i] = root_j;
            return true;
        }
        return false;
    }
};

class TdaEngine {
public:
    // Flat vector buffer layout for optimal cache-locality in WebAssembly memory
    static TdaResult inspectEmbeddingSpaceWasm(const std::vector<double>& flat_vectors, int n, int dim, double epsilon) {
        double start_time = emscripten_get_now();

        if (n == 0 || dim == 0) {
            return {0, 0, epsilon, 0, 0, false, 0.0};
        }

        // 1. Calculate Pairwise Distance Matrix & Filter Edges
        int edge_count = 0;
        DSU dsu(n);

        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                double dist_sq = 0.0;
                for (int d = 0; d < dim; ++d) {
                    double diff = flat_vectors[i * dim + d] - flat_vectors[j * dim + d];
                    dist_sq += diff * diff;
                }
                double dist = std::sqrt(dist_sq);

                if (dist <= epsilon) {
                    edge_count++;
                    dsu.unite(i, j);
                }
            }
        }

        // 2. Compute Betti-0 (Connected Components)
        int betti0 = 0;
        for (int i = 0; i < n; ++i) {
            if (dsu.find(i) == i) {
                betti0++;
            }
        }

        // 3. Euler Characteristic Approximation: Betti-1 ~ Edges - Vertices + Components
        int betti1 = std::max(0, edge_count - n + betti0);
        double elapsed_ms = emscripten_get_now() - start_time;

        return {
            n,
            dim,
            epsilon,
            betti0,
            betti1,
            betti1 > 0,
            elapsed_ms
        };
    }
};

// Emscripten Bindings Exposing C++ Core directly to WebAssembly JavaScript API
EMSCRIPTEN_BINDINGS(braidmesh_tda_module) {
    value_object<TdaResult>("TdaResult")
        .field("vectorCount", &TdaResult::vector_count)
        .field("dimensions", &TdaResult::dimensions)
        .field("epsilonThreshold", &TdaResult::epsilon_threshold)
        .field("betti0Clusters", &TdaResult::betti0_clusters)
        .field("betti1Loops", &TdaResult::betti1_loops)
        .field("hasVectorTraps", &TdaResult::has_vector_traps)
        .field("computeTimeMs", &TdaResult::compute_time_ms);

    register_vector<double>("VectorDouble");

    class_<TdaEngine>("TdaEngine")
        .constructor<>()
        .class_function("inspectEmbeddingSpaceWasm", &TdaEngine::inspectEmbeddingSpaceWasm);
}
