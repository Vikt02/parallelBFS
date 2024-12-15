#include <vector>
#include <tbb/tbb.h>

using namespace std;

int BLOCK = 100; // TODO

template<typename Func>
void p_for(int start, int end, Func func) {
    if (end - start <= BLOCK) {
        for (int i = start; i < end; i++) {
            func(i);
        }
        return;
    }
    int mid = (end + start) / 2;

    tbb::parallel_invoke(
            [start, mid, func] { p_for(start, mid, func); },
            [mid, end, func] { p_for(mid, end, func); }
    );
}

template<typename Func_map>
void p_map(vector<int> &a, int start, int end, Func_map func) {
    p_for(start, end, [&a, func](int i) {
        a[i] = func(i);
    });
}

void scan_serial(vector<int> &a, int start, int end, int delta) {
    a[start] += delta;
    for (int i = start + 1; i < end; i++) {
        if (i) {
            a[i] += a[i - 1];
        }
    }
}

int reduce_serial(vector<int> &a, int start, int end) {
    int sum = 0;
    for (int i = start; i < end; i++) {
        sum += a[i];
    }
    return sum;
}

int min(int a, int b) {
    if (a < b) {
        return a;
    } else {
        return b;
    }
}

void p_scan(vector<int> &a, int start, int end) {
    if (end - start <= BLOCK) {
        scan_serial(a, start, end, 0);
        return;
    }

    int sums_len = (end - start) / BLOCK + 1;
    vector<int> sums(sums_len);

    p_map(sums, 0, sums_len, [&a, start, end](int i)->int{
        return reduce_serial(a, start + i * BLOCK, start + min((i + 1) * BLOCK, end));
    });

    scan_serial(sums, 0, sums_len, 0);

    p_for(0, sums_len, [&a, start, end, &sums](int i)->void{
        int delta;
        if (i == 0) {
            delta = 0;
        } else {
            delta = sums[i - 1];
        }
        scan_serial(a, start + i * BLOCK, start + min((i + 1) * BLOCK, end), delta);
    });
}

template<typename Func_filter>
vector<int> p_filter(vector<int> &a, int start, int end, Func_filter predicate) {
    vector<int> filtered(end - start);

    p_for(start, end, [&a, &filtered, predicate](int i){
        if (predicate(a[i])) {
            filtered[i] = 1;
        } else {
            filtered[i] = 0;
        }
    });
    p_scan(filtered, 0, filtered.size());
    vector<int> res(filtered[filtered.size() - 1]);

    p_for(0, filtered.size(), [&filtered, &res, &a](int i){
        if ((i > 0 && filtered[i] != filtered[i - 1]) || (i == 0 && filtered[i])) {
            res[filtered[i] - 1] = a[i];
        }
    });

    return res;
}