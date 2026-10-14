# Phase 2a: Parallel Global Queries

## Introduction

In this phase, you will implement three classical graph algorithms (BFS, SSSP, and CC) using the Bulk Synchronous Parallel (BSP) execution model.

BSP divides the computation into rounds. In each round:
1. **Process** every vertex (in parallel when multiple threads are used).
2. **Synchronize**: wait for all threads to finish the round.
3. **Finalize** (single-threaded): combine per-thread results and set up state for the next round.

Repeat until a full round produces no updates.

You will implement a small BSP framework (a serial runner and a parallel runner built on `std::thread`) and plug in three algorithm callbacks.

## Requirements

1. **Graph Loading**: Extend the CSR representation from Phase 1 to carry:
   - Edge weights
   - A reverse CSR (in-edges) alongside the forward CSR

2. **BSP Framework**: Design a small framework that separates *what an algorithm does per vertex* from *how vertices are scheduled in a round*. Your framework should include:
   - An interface (e.g., an abstract base class) that an algorithm plugs into by providing a per-vertex callback, end-of-round bookkeeping, and a way to tell the runner when to stop.
   - A serial runner.
   - A parallel runner built on `std::thread`.

3. **Algorithms**: Implement the following three algorithms as callbacks to your framework.
   - **BFS**: shortest hop-distance from a source vertex.
   - **SSSP**: shortest weighted distance from a source vertex (Bellman-Ford style relaxation).
   - **CC**: weakly connected components via label propagation. Treat edges as undirected when propagating labels (each vertex adopts the minimum label among all its neighbors, in either direction).

4. **Correctness**: For each algorithm, the serial and parallel runs must produce identical per-vertex results.

5. **Performance Comparison**: Measure and compare execution times for the serial and parallel versions, and report speedup in the project report.

## Hints

See [shared input and answer file formats](common.md#file-formats).

### Data Structures & Interfaces

The following definitions are provided as a starting point. Feel free to adapt or redesign them to better suit your implementation.

```cpp
struct CsrGraph {
    uint32_t num_vertices;
    std::vector<uint32_t> offsets;      // out-edges, size = num_vertices + 1
    std::vector<uint32_t> edges;        // out-neighbor IDs
    std::vector<int> weights;           // edge weights (parallel to edges)
    std::vector<uint32_t> in_offsets;   // in-edges (reverse CSR)
    std::vector<uint32_t> in_edges;     // in-neighbor IDs
    std::vector<int> in_weights;        // reverse edge weights (parallel to in_edges)
};

class BspAlgorithm {
public:
    virtual ~BspAlgorithm() = default;

    // Returns true if another round should be run.
    virtual bool HasWork() const = 0;

    // Process one vertex. tid ∈ [0, nthreads). The serial runner calls with
    // tid=0; the parallel runner calls with each thread's ID.
    virtual void Process(int tid, uint32_t v, const CsrGraph& g) = 0;

    // End of round. Merge per-thread buffers, prepare state for next round.
    virtual void PostRound() = 0;
};

void BspSerial(const CsrGraph& g, BspAlgorithm& algo);
void BspParallel(const CsrGraph& g, BspAlgorithm& algo, int nthreads);
```

For example, a BFS subclass can declare the following methods; you implement their bodies:

```cpp
class Bfs : public BspAlgorithm {
public:
    Bfs(uint32_t num_vertices, uint32_t source, int nthreads);

    bool HasWork() const override;
    void Process(int tid, uint32_t v, const CsrGraph& g) override;
    void PostRound() override;

    // Add BFS state here: distances, previous-round snapshot, update flags.
};
```

`virtual ... = 0` declares a method that the subclass must implement. `override` lets the compiler check that the method's signature matches the interface.

Each of BFS, SSSP, and CC is implemented as a subclass of `BspAlgorithm` (e.g., `class Bfs : public BspAlgorithm`), then passed into a runner:

```cpp
Bfs bfs(g.num_vertices, /*source=*/0, /*nthreads=*/1);
BspSerial(g, bfs);

Bfs bfs_p(g.num_vertices, /*source=*/0, /*nthreads=*/4);
BspParallel(g, bfs_p, /*nthreads=*/4);
```

A minimal `BspSerial` is just a loop over the three virtual methods:

```cpp
void BspSerial(const CsrGraph& g, BspAlgorithm& algo) {
  while (algo.HasWork()) {
    for (uint32_t v = 0; v < g.num_vertices; v++) {
      algo.Process(/*tid=*/0, v, g);
    }
    algo.PostRound();
  }
}
```

`BspParallel` has the same outer structure. The only change is inside the round: instead of one thread iterating every vertex, partition `[0, num_vertices)` into `nthreads` contiguous ranges, spawn an `std::thread` per range (each calling `Process()` with its own `tid`), and wait for all threads to finish before calling `PostRound()`.

### Algorithm Design

Design `Process(tid, v, g)` so it only **reads** neighbor state and only **writes** `v`'s own state. The parallel runner assigns each vertex to exactly one thread (static partitioning of `[0, num_vertices)`), so `v`'s state is written by one thread per round.

Every round, every vertex `v` scans its in-edges (via the reverse CSR) and takes the best improvement, if any, from any in-neighbor. The algorithm is **done** when a full round produces no updates.

Initialize `state_` and `state_prev_` identically:

| Algorithm | Initial state |
| --- | --- |
| BFS | Source = 0; other vertices = -1 (unreachable) |
| SSSP | Source = 0; other vertices = infinity |
| CC | `state[v] = v` |

For BFS and SSSP, skip unreachable neighbors before computing a candidate. In BFS, a valid candidate improves an unreachable value of `-1`; do not compare `-1` as an ordinary distance.

```
Process(tid, v, g):
    best = current state of v
    updated = false
    for u in in_neighbors(v):
        candidate = value derived from state_prev[u]
                    //   BFS:  state_prev[u] + 1
                    //   SSSP: state_prev[u] + weight(u, v)
                    //   CC:   state_prev[u]
        if candidate improves best:
            best = candidate
            updated = true
    if updated:
        state of v = best
        tl_updated[tid] = 1
```

**Signaling convergence.** Each thread sets `tl_updated[tid] = 1` when it updates at least one vertex during the round. `PostRound()` reduces the per-thread flags into a single `any_updated` boolean; `HasWork()` returns that value, so the loop exits as soon as a round settles everything.

Initialize the per-thread flags to zero and `any_updated = true` so the first round runs. After reducing the flags in `PostRound()`, reset them to zero for the next round.

**Double buffering.** A vertex's value can be improved more than once (SSSP finds shorter paths, CC finds smaller labels). If thread A reads `state[u]` while thread B is simultaneously writing `state[u]`, the read is unsafe. To prevent this, keep two arrays: `state_` (live, written by `Process()`) and `state_prev_` (read by `Process()`, a snapshot of the values at the end of the previous round). `PostRound()` copies `state_` into `state_prev_` so the next round reads a stable view.

> **Note on CC**: CC computes weakly connected components, so labels should propagate across every edge regardless of direction. For CC, `Process()` must scan both in-neighbors and out-neighbors.

## Testing & Verification

Follow the [shared testing and verification instructions](common.md#testing--verification).

## Experiments

Run the following experiments on `soc-LiveJournal1-weighted.txt` and report in your project report. Use source vertex 0 for BFS and SSSP.

Follow the [shared experimental workflow and measurement definitions](common.md#overall-experimental-workflow).

1. **Serial execution time (seconds)** for BFS, SSSP, and CC.

2. **Speedup table**: For each algorithm, report the speedups below. The thread-count columns refer to the parallel runner with 1, 2, and 4 threads.

   | Algorithm | Serial | 1 thread | 2 threads | 4 threads |
   |---|---|---|---|---|
   | BFS | 1× | | | |
   | SSSP | 1× | | | |
   | CC | 1× | | | |

### Discussion

Answer the following questions in your project report:

1. Inside `Process(tid, v, g)`, which memory location does the function write to? Using the parallel runner's vertex partitioning, explain why two threads can never write to the same location in one round.

2. Compare the parallel speedup you measured for BFS, SSSP, and CC. Which algorithm scales best, and what property of the algorithm explains the difference?

3. In BFS, `dist_[v]` stores the current shortest hop distance from the source. BFS has two useful properties: (i) once `dist_[v]` is set, later rounds can never change it; (ii) in each round, v's newly-discovered in-neighbors all have the same distance. How could you use these facts to skip work? Why would the same tricks not work for SSSP or CC? (Implementing these optimizations is not required, but strongly encouraged: you should observe a significant speedup over the baseline BFS.)
