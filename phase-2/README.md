# Phase 2: Parallel Global Queries

Phase 2 combines the following three parts into one three-week project, **October 14–November 3**. Work through them in order and submit your code and the reports for all three parts at the final deadline.

We recommend completing one part per week: **2a in Week 1, 2b in Week 2, and 2c in Week 3**. These are suggested milestones, not separate submission deadlines.

You will implement multiple versions of each algorithm, such as BFS. Don't be intimidated by the number of versions: they will reuse much of the same code, with changes to how work is processed and updates are synchronized.

| Part | Description |
| --- | --- |
| [Phase 2a](01-pull-bsp.md) | Parallel Global Queries: BSP and pull-based BFS, SSSP, and CC |
| [Phase 2b](02-push-locks.md) | Push-based Graph Algorithms: dense worklists and destination locks |
| [Phase 2c](03-atomic-worklists.md) | Lock-free Push and Sparse Worklists: atomic updates and sparse frontiers |
| [Bonus](bonus.md) | Optional: switching between push and pull |

See [common.md](common.md) for the shared dataset, input/output formats, and testing instructions. Implementation requirements, experiments, and Discussion questions remain in each part's document.

## Submission

Submit code under `phase-2/code/` and **three reports**, one each for 2a, 2b, and 2c, under `phase-2/report/`. Use the [report template](../REPORT_TEMPLATE.md) for each report, including that part's experiments and Discussion answers. Describe shared data and experimental settings in the 2a report; the 2b and 2c reports may refer to it. Follow the [project rules](../README.md#project-rules).

For example:

```text
phase-2/report/
├── 2a.md          # or PDF
├── 2b.md
├── 2c.md
├── bonus.md       # only if attempting the bonus
└── images/
```

If you attempt the bonus, submit a separate bonus report explaining your strategy and experimental results.

**Bonus credit requires both documenting the work in your report and presenting your solution to me.** See the [bonus requirements](bonus.md).
