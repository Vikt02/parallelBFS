#include <vector>
#include <random>

using namespace std;

int count_point(int i, int j, int k, int n) {
    return i + j * n + k * n * n;
}

vector<vector<int>> generate_random_graph(int n, int p, int m = 100) {
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<int> distrib(0, 100);
    vector<vector<int>> v(n);
    for (int i = 0; i < n && m; i++) {
        for (int j = 0; j < n && m; j++) {
            if (distrib(gen) < p) {
                v[i].push_back(j);
                v[j].push_back(i);
                m--;
            }
        }
    }
    return v;
}

int x[] = {1, -1, 0, 0, 0, 0};
int y[] = {0, 0, 1, -1, 0, 0};
int z[] = {0, 0, 0, 0, 1, -1};

vector<vector<int>> generate_cube_graph(int n) {
    vector<vector<int>> v(n*n*n);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            for (int k = 0; k < n; k++) {
                int idx = count_point(i, j, k, n);
                v[idx].clear();
                for (int p = 0; p < 6; p++) {
                    if (i + x[p] < 0 || j + y[p] < 0 || k + z[p] < 0 || i + x[p] >= n || j + y[p] >= n || k + z[p] >= n) {
                        continue;
                    }
                    v[idx].push_back(count_point(i + x[p], j + y[p], k + z[p], n));
                }
            }
        }
    }

    return v;
}
