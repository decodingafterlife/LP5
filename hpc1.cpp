#include <iostream>
#include <vector>
#include <queue>
#include <cstdlib> // Required for rand()
#include <ctime>   // Required for time()
#include <omp.h>

using namespace std;

class Graph {
    int V;
    vector<vector<int>> adj;

public:
    Graph(int V) {
        this->V = V;
        adj.resize(V);
    }

    // Add edge to graph
    void addEdge(int u, int v) {
        adj[u].push_back(v);
        adj[v].push_back(u); // Undirected graph
    }

    // ---------------- SEQUENTIAL BFS ----------------
    void sequentialBFS(int start) {
        vector<bool> visited(V, false);
        queue<int> q;

        visited[start] = true;
        q.push(start);

        while (!q.empty()) {
            int node = q.front();
            q.pop();

            // cout << node << " "; // Commented out for accurate performance benchmarking

            for (int neighbor : adj[node]) {
                if (!visited[neighbor]) {
                    visited[neighbor] = true;
                    q.push(neighbor);
                }
            }
        }
    }

    // ---------------- PARALLEL BFS ----------------
    void parallelBFS(int start) {
        vector<bool> visited(V, false);
        queue<int> q;

        visited[start] = true;
        q.push(start);

        while (!q.empty()) {

            int current;

            #pragma omp parallel shared(q, visited)
            {
                // Only one thread removes element from queue
                #pragma omp single
                {
                    current = q.front();
                    q.pop();
                }

                // Parallel traversal of neighbors
                #pragma omp for
                for (int i = 0; i < adj[current].size(); i++) {

                    int neighbor = adj[current][i];

                    if (!visited[neighbor]) {

                        #pragma omp critical
                        {
                            if (!visited[neighbor]) {
                                visited[neighbor] = true;
                                q.push(neighbor);
                            }
                        }
                    }
                }
            }
        }
    }

    // ---------------- SEQUENTIAL DFS ----------------
    void sequentialDFSUtil(int node, vector<bool> &visited) {

        visited[node] = true;
        // cout << node << " "; // Commented out for accurate performance benchmarking

        for (int neighbor : adj[node]) {
            if (!visited[neighbor]) {
                sequentialDFSUtil(neighbor, visited);
            }
        }
    }

    void sequentialDFS(int start) {
        vector<bool> visited(V, false);
        sequentialDFSUtil(start, visited);
    }

    // ---------------- PARALLEL DFS ----------------
    void parallelDFSUtil(int node, vector<bool> &visited) {

        #pragma omp critical
        {
            visited[node] = true;
            // cout << node << " "; // Commented out for accurate performance benchmarking
        }

        #pragma omp parallel for
        for (int i = 0; i < adj[node].size(); i++) {

            int neighbor = adj[node][i];

            if (!visited[neighbor]) {
                parallelDFSUtil(neighbor, visited);
            }
        }
    }

    void parallelDFS(int start) {
        vector<bool> visited(V, false);
        parallelDFSUtil(start, visited);
    }
};

int main() {
    // Seed the random number generator so we get different graphs every time
    srand(time(0)); 

    int V, E;

    cout << "Enter number of vertices: ";
    cin >> V;

    cout << "Enter number of edges: ";
    cin >> E;

    Graph g(V);

    cout << "Generating " << E << " random edges...\n";

    for (int i = 0; i < E; i++) {
        // Pick two random nodes between 0 and V-1
        int u = rand() % V;
        int v = rand() % V;

        // Prevent a node from connecting to itself
        while (u == v) {
            v = rand() % V;
        }

        g.addEdge(u, v);
    }

    int startNode = 0; // We can default the starting node to 0
    cout << "Starting vertex: " << startNode << "\n";

    double start, end;

    // ---------------- SEQUENTIAL BFS ----------------
    cout << "\nRunning Sequential BFS... ";
    start = omp_get_wtime();
    g.sequentialBFS(startNode);
    end = omp_get_wtime();
    cout << "Time: " << (end - start) * 1000 << " ms" << endl;

    // ---------------- PARALLEL BFS ----------------
    cout << "Running Parallel BFS... ";
    start = omp_get_wtime();
    g.parallelBFS(startNode);
    end = omp_get_wtime();
    cout << "Time: " << (end - start) * 1000 << " ms" << endl;

    // ---------------- SEQUENTIAL DFS ----------------
    cout << "Running Sequential DFS... ";
    start = omp_get_wtime();
    g.sequentialDFS(startNode);
    end = omp_get_wtime();
    cout << "Time: " << (end - start) * 1000 << " ms" << endl;

    // ---------------- PARALLEL DFS ----------------
    cout << "Running Parallel DFS... ";
    start = omp_get_wtime();
    g.parallelDFS(startNode);
    end = omp_get_wtime();
    cout << "Time: " << (end - start) * 1000 << " ms" << endl;

    return 0;
}

/*

COMPILATION:

Linux:
g++ -fopenmp hpc1.cpp -o hpc1
export OMP_NUM_THREADS=4
./hpc1

Windows:
g++ -fopenmp hpc1.cpp -o hpc1.exe
$env:OMP_NUM_THREADS=4
.\hpc1.exe

*/