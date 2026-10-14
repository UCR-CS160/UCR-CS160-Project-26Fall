# Bonus: Switching Between Push and Pull

[Phase 2 overview](README.md) · [Atomic Updates and Sparse Worklists](03-atomic-worklists.md)

This optional task is worth **3 additional points** and is submitted with Phase 2 at the same deadline.

**To receive bonus credit, you must both submit a separate bonus report and present your solution to me.** Be prepared to explain your design, implementation, and experimental results, and answer questions about them.

## Introduction

Push is cheap when only a few vertices have anything to push. Pull's advantage is that each `v` is written by only one thread, so no locks are needed. BFS naturally starts with a tiny active set, grows to cover much of the graph in the middle rounds, then shrinks again, so the cheaper direction changes round by round.

## Goal and Requirements

Design an adaptive execution strategy that selects between push and pull for **BFS, SSSP, and CC**, building on your earlier implementations. Decide when and how to switch, and justify your strategy with experiments.

## Verification, Experiments, and Discussion

Verify against the provided answer files. On `soc-LiveJournal1-weighted.txt`, compare adaptive execution with pull-only and your best required push-only version, all at four threads. Identify the push variant used, report times and speedups relative to serial naive push, and include switching overhead in the timed run.

| Algorithm | Pull-only time (s) | Push-only time (s) | Adaptive time (s) |
| --- | ---: | ---: | ---: |
| BFS | | | |
| SSSP | | | |
| CC | | | |

In your report, explain your strategy and discuss where it helped or hurt, including its overhead and limitations.
