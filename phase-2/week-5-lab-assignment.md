This lab implements **parallel naive push BFS with atomic CAS**, a warmup for [Phase 2c](03-atomic-worklists.md). Reuse your Week 4 implementation and replace the destination locks with atomic updates.

## Push BFS with Atomic CAS

Run your BFS with `BspParallel` using 4 threads. Keep reading source distances from `dist_prev_` and reuse your snapshot and convergence bookkeeping.

- Use `std::atomic_ref<int>` and `compare_exchange_strong` from `<atomic>` (requires C++20). See [cas.cpp](cas.cpp) for a usage example.
- For each outgoing neighbor `v`, try to change its distance from -1 to the candidate hop distance `dist_prev_[u] + 1`. Only one thread can successfully discover `v`.
- Set `tl_updated_[tid] = 1` only when the CAS succeeds. For BFS, a failed CAS means the vertex has already been discovered, so no retry is needed.
- Within `Process()`, access the live destination distance through `std::atomic_ref`, including any reads. Skip source vertices whose snapshot distance is -1.

This is the **naive** version: each round processes every vertex reachable in the previous-round snapshot, without a worklist.

If you find this lab easy, we also recommend trying SSSP during the lab session, which involves more advanced use of CAS.

Run on `soc-LiveJournal1-weighted.txt` with source 0 and print the hop distance to vertex 50. Compare it with vertex 50's entry in the provided `BFS.txt` answer file.

Submit your atomic push BFS and parallel runner code and a screenshot of the results to Canvas before the lab ends.
