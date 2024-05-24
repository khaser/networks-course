#include <bits/stdc++.h>
#include <ratio>
#include <thread>
#include <utility>

#include "channel.h"

using namespace std;

struct Entry {
    int dest, next_hop, dist;
    bool operator == (const Entry &oth) const {
        return tie(dest, next_hop, dist) == tie(oth.dest, oth.next_hop, oth.dist);
    }
};

vector<Entry> merge(vector<Entry> a, const vector<Entry> &b, int from_b) {
    for (auto &entry : b) {
        auto it = find_if(a.begin(), a.end(), [&] (const Entry& e) {
            return e.dest == entry.dest;
        });
        if (it == a.end()) {
            a.push_back({entry.dest, from_b, entry.dist + 1});
        } else {
            if (it->dist > entry.dist) {
                *it = {entry.dest, from_b, entry.dist + 1};
            }
        }
    }
    return a;
}

int main(int argc, char* argv[]) {
    vector<vector<int>> g = {
        {1, 2, 3, 4},
        {0, 2},
        {0, 1, 3},
        {0, 2},
        {0, 5},
        {4}
    };
    int n = g.size();

    vector<vector<Entry>> tables(n);
    deque<BufferedChannel<pair<vector<Entry>, int>>> chans;
    for (int i = 0; i < n; ++i) {
        chans.emplace_back(n + 1);
        tables[i].push_back({i, i, 0});
        for (int j : g[i]) {
            tables[i].push_back({j, j, 1});
        }
    }

    mutex dump_mut;
    size_t dump_id = 0;
    auto dump = [&dump_mut, n, &tables, &dump_id] (std::string msg) {
        lock_guard lock(dump_mut);
        cout << "Dump " << dump_id++ << ' ' << msg << "\n";
        for (int i = 0; i < n; ++i) {
            cout << "Routing table for " <<  i << "\n";
            cout << setw(20) << internal << "Destination" <<
                    setw(20) << internal << "Next hop" <<
                    setw(20) << internal << "Distance\n";
            for (auto &entry : tables[i]) {
                cout << setw(20) << internal << entry.dest <<
                        setw(20) << internal << entry.next_hop <<
                        setw(20) << internal << entry.dist << "\n";
            }
        }
    };

    vector<jthread> threads;
    atomic_bool cancel = false;
    for (int i = 0; i < n; ++i) {
        threads.emplace_back([i, &dump, &tables, &chans, &g, &cancel] {
            try {
                while (!cancel) {
                    {
                        for (int j : g[i]) {
                            if (i != j) chans[j].Send({tables[i], i});
                        }
                    }
                    while (!cancel) {
                        optional<pair<vector<Entry>, int>> t = chans[i].Recv();
                        if (t) {
                            auto merged = merge(tables[i], t.value().first, t.value().second);
                            dump("Router " + to_string(i) + " received msg from router " + to_string(t.value().second));
                            if (merged != tables[i]) {
                                tables[i] = merged;
                                break;
                            }
                        }
                    }
                }
                } catch (...) { }
        });
    }

    this_thread::sleep_for(chrono::seconds(1));
    cancel = true;
    for (auto& chan : chans) {
        chan.Close();
    }


    return 0;
}
