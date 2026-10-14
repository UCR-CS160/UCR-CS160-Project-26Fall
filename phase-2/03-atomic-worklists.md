# Phase 2c: Lock-free Push and Sparse Worklists

## Introduction

Phase 2b made push parallel by wrapping every write to `state[v]` in a striped-lock critical section. This works, but it has two costs:

1. The body of the critical section is just a load, a compare, and a store. Wrapping it in `lock` / `unlock` adds tens of nanoseconds even when the lock is uncontended, often more than the work itself.
2. When many threads push to the same hub destination in the same round, they serialize on one mutex. A blocked thread can be parked by the OS, which is much slower than waiting on a hardware bus.

The push body does not actually need a mutex. It needs an *atomic* update of `state[v]`, which the CPU already provides as a single compare-and-swap (CAS) instruction. In this phase you will replace the lock with a CAS, and then go one step further by replacing the dense bitmap frontier with a **sparse worklist** that scales with `|F|` instead of `n`.

The `CsrGraph` and `BspAlgorithm` interfaces from Phase 2a/2b carry over. The sparse-worklist version doesn't fit the BSP runner's "scan `[0, n)` per round" shape, so it does its own loop (see Hints).

## Requirements

For each of **BFS**, **SSSP**, and **CC**, implement two new variants:

1. **Lock-free dense push.** Same algorithm and same dense bitmap as Phase 2b's parallel dense version, but every locked region is replaced by an atomic CAS on `state[v]`.

2. **Lock-free sparse push.** Replace the dense bitmap with a list of active vertex IDs. Each round iterates that list (size `|F|`), not `[0, n)`. Updates to `state[v]` still use atomic CAS.

**Correctness.** Per-vertex results must match Phase 2a/2b and the answer files (`BFS.txt`, `SSSP.txt`, `CC.txt`).

**Performance.** Measure execution time and report in your project report.

## Hints

### From locks to atomic CAS

The locked body in Phase 2b looked like this:

```
lock(locks[v & (K - 1)])
if candidate improves state[v]:
    state[v] = candidate
    in_next[v] = 1
unlock(...)
```

The only thing the lock prevents is two threads reading `state[v]` at the same time, both deciding to overwrite, and one update being lost. A **compare-and-swap** does exactly that in one hardware instruction: read a snapshot of `state[v]`, replace it with `candidate` only if `state[v]` is still equal to the snapshot. If another thread won, the CAS reports the new value and you decide whether to retry.

C++ exposes this through `std::atomic_ref<int>` (requires C++20) and the `compare_exchange_*` family. The right shape differs by algorithm:

- **BFS**: `dist[v]` is written at most once (`-1` to a positive value). One `compare_exchange_strong` from `-1` is enough. Success means "I am the discoverer," failure means someone else got there first.
- **SSSP, CC**: `state[v]` may be lowered more than once in the same round. The natural shape is a load + retry-CAS loop: keep CASing as long as your candidate is still smaller.

Read the live destination value through `atomic_ref.load()` as well; read source values from the previous-round snapshot.

### Sparse worklist

**Runner:** The atomic dense version still uses the existing BSP runner. The sparse version uses its own `Run()` loop to iterate the frontier instead of all vertex IDs. It still follows BSP rounds: process, synchronize, and prepare the next round.

The dense bitmap has one weakness: every round still scans `[0, n)` to find the active vertices. When `|F|` is small (early BFS rounds, late SSSP/CC rounds), most of that scan is `if !in_frontier[u]: return`.

A sparse worklist stores the frontier as a list of vertex IDs. Per round:

```
cur:    std::vector<uint32_t>            // current frontier
next_t: std::vector<uint32_t>[nthreads]  // each thread's per-thread output

partition cur by index across threads:
    for u in my slice of cur:
        for v in out_neighbors(u):
            // atomic CAS on state[v]; append v to next_t[tid] using the dedup rules below

concat all next_t into the new cur:
    1. prefix sum over the T buffer sizes (sequential, T values)
    2. each thread memcpy's its buffer into its slot of cur, in parallel

while cur.size() > 0
```

A few things to notice:

- The outer loop is over `cur`, so the per-round cost is `O(|F| + edges leaving F)` instead of `O(n + edges leaving F)`.
- Per-thread output buffers + concat avoid contention on a shared queue.

### Dedup in the sparse path

The dense bitmap is naturally deduplicated: two threads writing `1` to `in_next[v]` is idempotent.

The sparse worklist is **not** naturally deduplicated. If `v` ends up in two different per-thread buffers in one round, the next round will visit it twice and do redundant work.

The invariant you need: **for each v that became active this round, exactly one thread appends v to its buffer.**

- **BFS**: trivial. The discover-once CAS (`-1 -> du + 1`) succeeds for exactly one thread. Append on CAS success and you are done.
- **SSSP, CC**: harder. Many threads can each lower `state[v]` in the same round (each one wins a CAS against the previous winner). You don't want one append per CAS winner.

  One trick is **snapshot dedup**. At the start of the round, `state_prev[v]` holds the value as of the end of the previous round. The thread whose successful CAS replaced exactly `state_prev[v]` is the **first** to lower `v` this round; later CAS winners see a different `expected` value. Only the first one appends. At the end of the round, after all workers finish and their buffers are concatenated into the **new `cur`**, copy `state_prev[v] = state[v]` for each `v in cur`, in parallel, cost `O(|F|)`.

### Initial frontier (sparse)

- **BFS, SSSP**: `cur = {source}`.
- **CC**: `cur = {0, 1, ..., n - 1}`. Same reasoning as Phase 2b: every label is "new" in round 0. The first round is dense by construction; the sparse representation only starts paying off later.

## Testing & Verification

Follow the [shared dataset, file formats, and verification instructions](common.md).

## Experiments

Run the following experiments on `soc-LiveJournal1-weighted.txt` and report in your project report. Use source vertex 0 for BFS and SSSP.

Follow the [shared experimental workflow and measurement definitions](common.md#overall-experimental-workflow).

1. **Speedup table**: For each algorithm, use Phase 2b's **serial lock-free naive push** time as the 1× baseline and report the speedups below.

   | Algorithm | 2b: Dense parallel / 4t (lock, K=1024) | 2c: Dense parallel / 4t (atomic) | 2c: Sparse parallel / 4t (atomic) |
   |---|---|---|---|
   | BFS  | | | |
   | SSSP | | | |
   | CC   | | | |

### Discussion

Answer the following in your project report:

1. Compare your atomic dense version against Phase 2b's locked dense version for BFS, SSSP, and CC. Which algorithm benefits the most from dropping the lock, and what property of the algorithm explains the difference?

2. Compare your atomic sparse version against the atomic dense version for BFS, SSSP, and CC. Which version wins for each algorithm? Can you explain the difference?

3. Print the worklist size (`|F|`) per round for the sparse variant of each algorithm. How does it evolve over time? When is the sparse worklist more efficient than the dense bitmap? Does that give you any ideas for the bonus task?

4. **Optional experiment (recommended):** Here is another dataset: [roadNet-CA.txt](https://snap.stanford.edu/data/roadNet-CA.html). It has a very different structure from `soc-LiveJournal1-weighted.txt` ([original link](https://snap.stanford.edu/data/soc-LiveJournal1.html)). This experiment is not required, but you are encouraged to try running your implementations on it. The format differs from the LiveJournal file in two ways:
   - **Undirected**: the downloaded file already contains both `(u, v)` and `(v, u)` for each connection. Store each input line once in the forward CSR; do not add extra reverse-edge records.
   - **Unweighted**: the file has no weight column. Generate one random weight in `[1, 400]` per undirected connection and assign it to both directions. Reuse the same generated weighted graph for all implementations you compare.

   How do the speedups compare with the previous dataset? Can you explain any differences based on the graph structure?
