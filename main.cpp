#include <iostream>
#include <queue>
#include "graph.h"
#include "parallel_base.h"

int const P = 500;
int const X = P;
int const Y = P;
int const Z = P;
int const N = X * Y * Z * 2;
int const THREADS = 4;

queue<int> q;
vector<int> dist(N);
vector<vector<int>> v(N);
vector<int> front(N);
vector<int> part(N);
vector<int> new_front(N);

// for test
vector<int> check_dist(N);

void seq_BFS(int s) {
    dist[s] = 0;
    q.push(s);

    while (!q.empty()) {
        int cur = q.front();
        q.pop();
        for (int i = 0; i < v[cur].size(); i++) {
            int u = v[cur][i];
            if (dist[u] > dist[cur] + 1) {
                dist[u] = dist[cur] + 1;
                q.push(u);
            }
        }
    }
}

void par_BFS(int s) {
    vector<atomic<int>> used(N);
    used[s] = 1;
    dist[s] = 0;

    front[0] = 0;
    int size = 1;
    int cur_dist = 0;

    while (size != 0) {
        cur_dist++;
        p_for(0, size, [](int i)->void{
            part[i] = v[front[i]].size();
        });

        p_scan(part, 0, size);

        int new_size = part[size - 1];
        new_front = vector<int>(new_size, -1);

        p_for(0, size, [cur_dist, &used](int i)->void {
            int f = front[i];
            int offset;
            if (i == 0) {
                offset = 0;
            } else {
                offset = part[i - 1];
            }

            p_for(0, v[f].size(), [f, cur_dist, offset, &used](int j)->void {
                int u = v[f][j];
                int zero = 0;
                int one = 1;
                if (used[u].compare_exchange_strong(zero, one)) {
                    dist[u] = cur_dist;
                    new_front[offset + j] = u;
                }
            });
        });

        front = p_filter(new_front, 0, new_size, [](int x)->bool {
            return x != -1;
        });
        size = front.size();
    }
}

void clear_and_prepare(bool clear_vector) {
    dist.clear();
    dist = vector<int>(N, INT_MAX);
    front = vector<int>(N);
    part = vector<int>(N);
    if (clear_vector) {
        v.clear();
    }
}

void perform() {
    v = generate_cube_graph(P);
    cout << "Start perform tests:\n";
    double sum1 = 0;
    double sum2 = 0;

    for (int i = 0; i < 5; i++) {
        auto start = std::chrono::high_resolution_clock::now();
        seq_BFS(0);
        auto finish = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> result = finish - start;
        sum1 += result.count();
        std::cout << "Seq run " << i << ": " << result.count() << " seconds.\n";
        clear_and_prepare(false);
    }
    cout << "Seq mid " << sum1 / 5 << "\n";

    for (int i = 0; i < 5; i++) {
        auto start1 = std::chrono::high_resolution_clock::now();
        par_BFS(0);
        auto finish1 = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> result1 = finish1 - start1;
        sum2 += result1.count();
        std::cout << "Par run " << i << ": " << result1.count() << " seconds.\n";
        clear_and_prepare(false);
    }
    cout << "Par mid " << sum1 / 5 << "\n";

    cout << "ratio: " << sum1 / sum2;

}

void correctness() {
    cout << "Start correctrness tests: \n";
    clear_and_prepare(true);
    int n = 10;
    v = generate_random_graph(n, 100, n * n);
    seq_BFS(0);
    for (int i = 0; i < n; i++) {
        check_dist[i] = dist[i];
        dist[i] = INT_MAX;
    }
    par_BFS(0);
    for (int i = 0; i < n; i++) {
        assert(check_dist[i] == dist[i]);
    }

    clear_and_prepare(true);
    n = 100;
    v = generate_random_graph(n, 50, n * n);
    seq_BFS(0);
    for (int i = 0; i < n; i++) {
        check_dist[i] = dist[i];
        dist[i] = INT_MAX;
    }
    par_BFS(0);
    for (int i = 0; i < n; i++) {
        assert(check_dist[i] == dist[i]);
    }

    clear_and_prepare(true);
    n = 100;
    v = generate_random_graph(n, 10, n * n);
    seq_BFS(0);
    for (int i = 0; i < n; i++) {
        check_dist[i] = dist[i];
        dist[i] = INT_MAX;
    }
    par_BFS(0);
    for (int i = 0; i < n; i++) {
        assert(check_dist[i] == dist[i]);
    }
    cout << "Correctness passed!\n";
}

int main() {
    tbb::global_control control(tbb::global_control::max_allowed_parallelism, THREADS);

    correctness();
    perform();
    return 0;
}