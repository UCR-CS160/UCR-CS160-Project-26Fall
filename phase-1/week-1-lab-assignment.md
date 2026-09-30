Implement `LoadGraph` to read the provided [soc-Slashdot0902.txt](https://drive.google.com/drive/folders/1Cr4QkLBpWa3Gp0u-9YWNE9voH9Dz4INg) edge list and build a Compressed Sparse Row (CSR) graph.

```cpp
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

struct CSRGraph {
    int num_vertices;
    std::vector<int> offsets;  // size: num_vertices + 1
    std::vector<int> edges;    // destination IDs, grouped by source vertex
};

CSRGraph LoadGraph(const char *filename);
```

Each non-comment line in the file contains a directed edge `src dst`. Skip lines beginning with `#` and store every edge, including self-loops. For vertex `v`, its outgoing neighbors occupy `edges[offsets[v] ... offsets[v + 1])`. Its out-degree is `offsets[v + 1] - offsets[v]`.

**File-reading hint:** `std::getline` reads one line at a time. For example:

```cpp
std::ifstream input(filename);
std::string line;
while (std::getline(input, line)) {
    // Skip comments, then parse src and dst from this line.
}
```

After skipping comments, you can use `std::istringstream` to read the two integers from `line`.

For example, consider this edge list:

```text
0 1
0 2
1 2
```

One valid CSR representation is:

```text
num_vertices = 3
offsets      = [0, 2, 3, 3]
edges        = [1, 2, 2]
```

The neighbors of vertex 0 are `edges[0 ... 2) = [1, 2]`; vertex 2 has no outgoing neighbors because `offsets[2] == offsets[3]`.

To inspect a loaded graph, you can print a vertex's neighbors using the CSR arrays:

```cpp
CSRGraph g = LoadGraph("soc-Slashdot0902.txt");
int v = 6;
for (int i = g.offsets[v]; i < g.offsets[v + 1]; ++i) {
    std::cout << g.edges[i] << ' ';
}
```

For the provided dataset, check that `num_vertices == 82168`, `edges.size() == 948464`, and vertex 6 has 23 outgoing edges. Submit your `LoadGraph` implementation and an image of its execution result on Canvas; show these checks and the neighbors of vertex 6.
