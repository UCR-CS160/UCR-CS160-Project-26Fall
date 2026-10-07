In Phase 1, the graph is read-only after loading, and each query produces its own result. In this lab, implement the concurrent task runner from the [Phase 1 project](README.md). You can reuse it in your project submission.

Use the following interface (or the equivalent interface in your project):

```cpp
#include <functional>
#include <string>
#include <vector>

struct CSRGraph {
    int num_vertices;
    std::vector<int> offsets;
    std::vector<int> edges;
};

using QueryCallback = std::function<std::string(const CSRGraph&, int src, int K)>;

struct QueryTask {
    int src;
    int K;
    QueryCallback cb;
    std::string result;
};

void RunTasksParallel(const CSRGraph& g, std::vector<QueryTask>& tasks,
                      int num_threads);
```

**Your task:** Implement `RunTasksParallel` using `std::thread`. Create `num_threads` workers and divide the tasks between them. Each task must call `tasks[i].cb(g, tasks[i].src, tasks[i].K)` exactly once and store the returned string in `tasks[i].result`. Keep results at their original task indices, and join all workers before returning. See [std_thread.cpp](std_thread.cpp) for the basic thread API.

You can test your runner without finishing the K-hop queries. This small CSR graph has four vertices:

```cpp
CSRGraph g{4, {0, 2, 3, 4, 4}, {1, 2, 3, 3}};

QueryCallback out_degree = [](const CSRGraph& graph, int src, int) {
    return std::to_string(graph.offsets[src + 1] - graph.offsets[src]);
};

std::vector<QueryTask> tasks;
for (int src = 0; src < 4; ++src) {
    tasks.push_back({src, 0, out_degree, ""});
}
```

Run these four tasks with two threads. In task order, the results should be `2, 1, 1, 0`. Run the same callbacks sequentially and check that the results match.

Submit your `RunTasksParallel` implementation and a screenshot of the results to Canvas. Note: the full Phase 1 project is submitted separately to your GitHub repository.
