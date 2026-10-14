# Phase 2: Shared Data and Verification

[Phase 2 overview](README.md)

## File Formats

### Weighted Edge List Format

The input graph is a tab-separated text file with three columns:

```
src	dst	weight
```

Lines starting with `#` are comments. Example:

```
# FromNodeId	ToNodeId	Weight
0	1	194
0	2	221
1	3	76
2	3	175
```

### Answer File Format

Each algorithm writes one line per vertex, sorted by vertex ID. The two columns are separated by a space.

- **BFS**: `vertex_id hop_distance`. Hop distance from the source, or `-1` if unreachable.
- **SSSP**: `vertex_id weighted_distance`. Shortest weighted distance from the source, or `-1` if unreachable.
- **CC**: `vertex_id component_id`. The component ID is the smallest vertex ID in the weakly connected component containing `vertex_id`.

## Testing & Verification

1. **Small test cases**: Build small graphs (5–10 vertices) where you can manually compute expected BFS distances, SSSP shortest paths, and CC labels. Include:
   - Disconnected components (for CC)
   - Cycles (to test convergence)
   - Varying edge weights (to distinguish SSSP from BFS)

2. **Serial-parallel consistency**: Verify that serial and parallel produce identical per-vertex results for all three algorithms.

3. **Provided dataset**: Download `soc-LiveJournal1-weighted.txt` from the [shared folder](https://drive.google.com/drive/folders/1MR0FuR0UxRCIFYBpEQ9e4Bx8s4qA_U7F?usp=sharing). This is a directed social network graph:
   - 4,847,571 vertices, 68,993,773 edges
   - Tab-separated, with integer edge weights in [1, 400]
   - Use `head -n 5 <file-name>` to have a quick look at the format of the provided dataset.
   - Add it (and the answer files) to `.gitignore` so it is not committed to GitHub.

4. **Answer files**: The shared folder also contains expected answer files (`BFS.txt`, `SSSP.txt`, `CC.txt`) for the provided dataset. Compare your output against these files to verify correctness. BFS and SSSP use source vertex 0.

## Overall Experimental Workflow

Use the same machine and compiler settings for all versions you compare, and initialize fresh algorithm state before each run. Measure elapsed time from the initialized state until convergence, including thread management and end-of-round work. Exclude graph loading, answer checking, and file output. Record your experimental settings and thread counts in the report.

Measure one **complete algorithm run**, including all rounds. Follow this workflow for each algorithm, version, and thread configuration required by the part's Experiments section:

```text
graph = LoadGraph("soc-LiveJournal1-weighted.txt")

for each algorithm (BFS, SSSP, CC):
    for each required version and thread configuration:
        initialize fresh algorithm state
        start = now()
        run the chosen runner or sparse Run() until convergence
        execution_time = now() - start
        verify all vertex results against the expected answers
        write the answer file
        record the version, thread count, and execution time
```

### Measurements and Baselines

For each run, report the algorithm, version, thread count, **execution time in seconds**, and **speedup**. Measure each algorithm's baseline on the same graph and with the same experimental settings as the versions being compared:

| Part | Speedup baseline |
| --- | --- |
| 2a | Serial pull time, measured with `BspSerial` |
| 2b and 2c | Serial lock-free naive push time from 2b |

Compute `speedup = baseline_time / execution_time`. The baseline is 1×, and a value below 1 means slower than the baseline. The speedup tables in each part contain these factors; also report the measured execution times.

For example, if BFS serial lock-free naive push takes 500 ms and a parallel version takes 200 ms, its speedup is `500 / 200 = 2.5×`; if it takes 800 ms instead, the speedup is `500 / 800 = 0.63×`.

