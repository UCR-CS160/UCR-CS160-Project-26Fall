This lab completes two parts of [Phase 2a](01-pull-bsp.md): loading a weighted graph with forward and reverse CSR, and running serial BFS through the BSP interface.

## 1. Weighted Graph with Outgoing and Incoming Edges

Extend your Phase 1 loader to read `src dst weight` and build the `CsrGraph` defined in [Phase 2a](01-pull-bsp.md#data-structures--interfaces). The dataset and input format are in [common.md](common.md).

For each directed edge `u → v` with weight `w`:

- Store `v` and `w` in the forward CSR, grouped by source `u`.
- Store `u` and `w` in the reverse CSR, grouped by destination `v`.

The graph remains directed; the reverse CSR provides an index of incoming edges. Keep the neighbor and weight arrays aligned in both representations.

## 2. Serial BFS Using BspAlgorithm

Implement `Bfs` as a subclass of `BspAlgorithm`, following the pull-style design in [Phase 2a](01-pull-bsp.md#algorithm-design). Implement `HasWork()`, `Process()`, and `PostRound()`. Use the `BspSerial` runner shown in that document.

BFS computes shortest **hop distances**, so it ignores the edge weights. Initialize the source distance to 0 and all other distances to -1. Stop when a full round produces no updates.

Start with the following class skeleton and fill in each `TODO` before running:

```cpp
class Bfs : public BspAlgorithm {
public:
    Bfs(uint32_t num_vertices, uint32_t source, int nthreads) {
        // TODO: initialize the distance arrays and update flags.
    }

    bool HasWork() const override {
        // TODO: return whether another round should run.
    }

    void Process(int tid, uint32_t v, const CsrGraph& g) override {
        // TODO: update vertex v using its incoming neighbors' snapshot values.
    }

    void PostRound() override {
        // TODO: combine and reset update flags, then update the snapshot.
    }

    int distance(uint32_t v) const {
        // TODO: return the hop distance to v, or -1 if unreachable.
    }

private:
    std::vector<int> dist_;             // current hop distances
    std::vector<int> dist_prev_;        // previous-round snapshot
    std::vector<uint8_t> tl_updated_;   // one update flag per thread; serial uses index 0
    bool any_updated_ = true;          // run the first round
};
```

In the constructor, size both distance arrays to `num_vertices` and initialize them to -1, then set the source distance to 0 in both. Allocate `nthreads` update flags initialized to zero. `Process()` reads the snapshot and updates `dist_[v]`; `PostRound()` combines and resets the flags, then updates the snapshot. `HasWork()` returns `any_updated_`.

After loading the graph into `g`, run BFS as follows:

```cpp
Bfs bfs(g.num_vertices, /*source=*/0, /*nthreads=*/1);
BspSerial(g, bfs);
```

Run on `soc-LiveJournal1-weighted.txt` with source 0 and print the hop distance to vertex 50. Compare it with vertex 50's entry in the provided `BFS.txt` answer file.

Submit your graph-loading and BFS code and a screenshot of the results to Canvas before the lab ends. Show the loaded graph's vertex count, outgoing and incoming edge counts, and the BFS hop distance from source 0 to vertex 50.
