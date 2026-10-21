This lab implements **parallel naive push BFS with locks**, requirement 3 of [Phase 2b](02-push-locks.md). Reuse your graph loader, BSP interface, and BFS state from Phase 2a.

## Push BFS with Locks

Implement a push BFS subclass of `BspAlgorithm` and run it with `BspParallel` using 4 threads.

In `Process(tid, u, g)`, read `dist_prev_[u]`. If it is -1, return. Otherwise, traverse `u`'s outgoing edges and try to update each neighbor `v` with the candidate hop distance `dist_prev_[u] + 1`.

- Use a pool of 1024 mutexes, mapping destination `v` to `locks[v % 1024]`.
- Use `std::lock_guard<std::mutex>` from `<mutex>` to protect both the check and the update of `dist_[v]`. Within `Process()`, guard every read and write of the live destination distance with this lock. See [lock.cpp](lock.cpp) for a usage example.
- If an update occurs, set `tl_updated_[tid] = 1`. Reuse the snapshot and convergence bookkeeping from your pull BFS.

This is the **naive** version: each round processes every vertex reachable in the previous-round snapshot, without a worklist.

See [Phase 2b](02-push-locks.md#algorithm-design-naive-push) for the algorithm design and [Phase 2a](01-pull-bsp.md#data-structures--interfaces) for the parallel runner.

Run on `soc-LiveJournal1-weighted.txt` with source 0 and print the hop distance to vertex 50. Compare it with vertex 50's entry in the provided `BFS.txt` answer file.

Submit your push BFS and parallel runner code and a screenshot of the results to Canvas before the lab ends.
