# Phase 2b: Push-based Graph Algorithms

## Introduction

Phase 2a implemented BFS, SSSP, and CC in **pull style**: each round, every vertex `v` scanned its in-edges and took the best improvement from any in-neighbor. Pull has a convenient property: the thread that owns `v` is the only writer of `state[v]`, so different threads never collide on the same destination.

In this phase you will re-implement the same three algorithms in **push style** on the same BSP framework. In push, each active vertex `u` walks its **out-edges** and **writes to** each out-neighbor `v`:

- **BFS**: set `dist[v] = dist[u] + 1` if `v` is not yet discovered.
- **SSSP**: set `dist[v] = min(dist[v], dist[u] + weight(u, v))`.
- **CC**: set `comp[v] = min(comp[v], comp[u])` (undirected: iterate both in- and out-neighbors).

Push has an advantage over pull: you only visit edges that matter, i.e., the edges leaving vertices whose state changed last round. But it also introduces a new problem: two different source vertices handled by two different threads can push to the **same** destination `v` at the same time. You must coordinate those writes.

## Requirements

Graph loading, file formats, and the `CsrGraph` / `BspAlgorithm` interfaces from Phase 2a carry over unchanged.

Implement each of **BFS**, **SSSP**, and **CC** four times. Don't be alarmed by the count: the four versions share most of their code, so after you write (1) the other three are mostly small deltas (add a worklist; add locks; add both).

1. **Serial lock-free, naive push.** Push-based, single-threaded, no worklist, no locks. Every round, every vertex with a known state pushes to its out-neighbors. This is your simplest reference implementation.

2. **Serial lock-free, dense worklist.** Adds a dense bitmap frontier (e.g., `std::vector<uint8_t>`) to (1). Each round only processes vertices whose state changed in the previous round. Still single-threaded, still no locks.

3. **Parallel naive push.** The same algorithm as (1), run under the Phase 2a parallel runner. Because multiple threads can write to the same `v`, this version must use locks to prevent data races.

4. **Parallel push with a dense worklist.** The same algorithm as (2), run under the parallel runner. Still uses locks.

**Correctness.** All four versions of each algorithm must produce identical per-vertex results, and identical to the Phase 2a answer files (`BFS.txt`, `SSSP.txt`, `CC.txt`).

**Performance.** Measure and compare execution time for all four versions. Report in your project report (see Experiments below).

## Hints

### Why push races, and pull does not

In pull, `Process(tid, v, g)` reads from in-neighbors and writes `state[v]`. The parallel runner assigns each `v` to exactly one thread, so `state[v]` has a single writer per round.

In push, `Process(tid, u, g)` reads from `u` and writes `state[v]` for each out-neighbor `v`. The runner still partitions by the *argument* to `Process`, but now that argument is the **source** `u`, not the destination. Nothing prevents two sources (handled by two different threads) from pointing at the same `v`. Without synchronization, the concurrent writes race: updates can be lost, and the read-then-compare-then-write sequence can interleave incorrectly.

### Protecting writes with a striped lock pool

You need a mutex associated with the destination `v`. One mutex per vertex is cleanest but wasteful: `sizeof(std::mutex)` is tens of bytes, so millions of vertices would use hundreds of megabytes just for locks.

A **striped lock pool** is a better fit: allocate a fixed pool of `K` mutexes (e.g., `K = 1024`) and map `v` to `locks[v % K]`. Memory stays constant in `K`, at the cost that two unrelated destinations occasionally share a mutex. Pick `K` as a power of two so `v % K` can be replaced with `v & (K - 1)`.

### Algorithm design (naive push)

As in Phase 2a, keep two arrays (`state_` and `state_prev_`) so reads of a neighbor's state don't race with writes. `PostRound()` copies `state_` into `state_prev_` at the end of each round so the next round starts from a stable snapshot.

Pseudocode for one round of parallel naive push (requirement 3):

```
Process(tid, u, g):
    if u has no known state: return
    for v in out_neighbors(u):
        candidate = derived from state_prev[u]
                    //   BFS:  state_prev[u] + 1
                    //   SSSP: state_prev[u] + weight(u, v)
                    //   CC:   state_prev[u]
        lock(locks[v & (K - 1)])
        if candidate improves state[v]:
            state[v] = candidate
            tl_updated[tid] = 1
        unlock(locks[v & (K - 1)])
```

Termination works the same as Phase 2a: each thread sets `tl_updated[tid]` when it writes something, `PostRound()` reduces the flags, `HasWork()` returns the reduced bit.

### Dense-worklist push (requirements 2 and 4)

The worklist is represented as a dense byte bitmap (e.g., `std::vector<uint8_t>`). You need two of them:

- `in_frontier[v]`: is `v` in the current round's frontier?
- `in_next[v]`: will `v` be in the next round's frontier?

```
Process(tid, u, g):
    if !in_frontier[u]: return
    for v in out_neighbors(u):
        candidate = derived from state_prev[u]
        lock(locks[v & (K - 1)])
        if candidate improves state[v]:
            state[v] = candidate
            in_next[v] = 1
            tl_updated[tid] = 1
        unlock(locks[v & (K - 1)])

PostRound():
    reduce tl_updated flags as in Phase 2a
    state_prev = state
    swap(in_frontier, in_next)
    fill(in_next, 0)
```

**Initial frontier.**

- **BFS, SSSP**: the frontier starts with only the source vertex.
- **CC**: the frontier starts with **every** vertex. Every label is "new" in round 0 because the graph has just been initialized with `comp[i] = i`. The frontier will shrink on its own in subsequent rounds.

> **Note on CC.** CC propagates labels across edges in both directions, so push must walk both out-neighbors and in-neighbors of each active `u`.

## Testing & Verification

Follow the [shared dataset, file formats, and verification instructions](common.md).

## Experiments

Run the following experiments on `soc-LiveJournal1-weighted.txt` and report in your project report. Use source vertex 0 for BFS and SSSP.

Follow the [shared experimental workflow and measurement definitions](common.md#overall-experimental-workflow).

1. **Serial lock-free execution time** for BFS, SSSP, and CC. Report both the naive-push and dense-worklist variants.

2. **Speedup table**: For each algorithm, report the speedups below. The **Pull** columns refer to your Phase 2a implementation: re-run it on the same dataset.

   | Algorithm | Serial LF, naive | Serial LF, dense | Naive parallel / 1t | Naive parallel / 4t | Dense parallel / 1t | Dense parallel / 4t | Pull / 1t | Pull / 4t |
   |---|---|---|---|---|---|---|---|---|
   | BFS | 1× | | | | | | | |
   | SSSP | 1× | | | | | | | |
   | CC | 1× | | | | | | | |

### Discussion

Answer the following in your project report:

1. Compare (a) the serial lock-free naive push against (b) the 1-thread naive parallel push. They run the same algorithm on a single thread, but (b) also acquires and releases a lock for every push. Are the two times the same? If not, where does the difference come from?

2. Compare (a) the serial lock-free naive push against (b) the serial lock-free dense worklist. Neither takes a lock, so any time difference is purely algorithmic. For BFS specifically, explain why (b) visits strictly fewer edges than (a). Does the same saving apply to SSSP and CC?

3. Compare your **parallel** versions (3, 4) against the **serial lock-free** versions (1, 2). For SSSP and CC, 4 threads with locks may be **slower** than 1 thread without locks; this is expected. Share your own analysis of the numbers you measured: which scaling patterns match your intuition, which surprise you, and what do you think is going on inside the parallel version? (Phase 2c will revisit these with different synchronization techniques.)

4. A **hub** is a vertex with unusually high in-degree, so many other vertices point at it. First verify that hubs exist in `soc-LiveJournal1-weighted.txt`: report the **top-3 in-degrees**. Then answer: when a hub appears as a destination, many threads push to it in the same round and serialize on the hub's lock. Does this problem go away if you enlarge the striped pool? Does it go away if you give every vertex its own mutex? Explain why hub contention is inherent to push-based parallelism.

5. Run the **Dense parallel** version with three different lock-pool sizes: `K = 1024`, `K = 16384`, and `K = 262144`. Fill in:

   | Algorithm | K = 1024 | K = 16384 | K = 262144 |
   |---|---|---|---|
   | BFS  | | | |
   | SSSP | | | |
   | CC   | | | |

   For each algorithm, which pool size is fastest? Why is "more locks" not always better?
