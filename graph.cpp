#include <bits/stdc++.h>
using namespace std;

static mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());
double drand() { return uniform_real_distribution<double>(0.0,1.0)(rng); }

struct Node {
    double x, y;
    double opinion;
    double stubbornness;
    double influence;
    double exposure;
};

// -------------------------------------------
// ==========  DSU (Union–Find)  ============
// -------------------------------------------
struct DSU {
    vector<int> parent, sz;

    DSU(int n=0){ init(n); }

    void init(int n){
        parent.resize(n);
        sz.assign(n,1);
        iota(parent.begin(), parent.end(), 0);
    }

    int find(int x){
        if(parent[x]==x) return x;
        return parent[x] = find(parent[x]);
    }

    void unite(int a, int b){
        a = find(a);
        b = find(b);
        if(a == b) return;
        if(sz[a] < sz[b]) swap(a,b);
        parent[b] = a;
        sz[a] += sz[b];
    }
};
// -------------------------------------------

int main() {
    int N = 1000;
    int m = 2;
    int steps = 300;

    double tolerance = 0.25;
    double stubborn_growth = 0.003;
    double ghetto_tolerance = 0.1;

    double MAX_RADIUS = 0.12;

    int num_pos = 40;
    int num_neg = 40;
    int num_neutral = 0;

    vector<Node> nodes(N);
    vector<vector<int>> adj(N);

    // ===== 1. Create node positions =====
    for(int i=0;i<N;i++){
        nodes[i].x = drand();
        nodes[i].y = drand();
        nodes[i].opinion = drand()*0.2 - 0.1;
        nodes[i].stubbornness = 0.4 + drand()*0.4;
        nodes[i].influence = 0.5 + drand()*0.5;
        nodes[i].exposure = 0.7 + drand()*0.3;
    }

    // ===== 2. Spatial BA Graph =====
    for(int i=1;i<N;i++){
        vector<pair<double,int>> candidates;

        for(int j=0;j<i;j++){
            double dx = nodes[i].x - nodes[j].x;
            double dy = nodes[i].y - nodes[j].y;
            double dist = sqrt(dx*dx + dy*dy);

            if(dist <= MAX_RADIUS){
                int deg = adj[j].size()+1;
                candidates.push_back({deg, j});
            }
        }

        if(candidates.empty()){
            vector<pair<double,int>> tmp;
            for(int j=0;j<i;j++){
                double dx = nodes[i].x - nodes[j].x;
                double dy = nodes[i].y - nodes[j].y;
                tmp.push_back({sqrt(dx*dx + dy*dy), j});
            }
            sort(tmp.begin(), tmp.end());
            for(int k=0;k < min(20,(int)tmp.size()); k++)
                candidates.push_back({1, tmp[k].second});
        }

        sort(candidates.begin(), candidates.end(),
             [&](auto &a, auto &b){ return a.first > b.first; });

        for(int k=0;k<m && k < (int)candidates.size(); k++){
            int v = candidates[k].second;
            adj[i].push_back(v);
            adj[v].push_back(i);
        }
    }

    // ===== 3. Influencers =====
    vector<int> idx(N);
    iota(idx.begin(), idx.end(), 0);
    shuffle(idx.begin(), idx.end(), rng);

    vector<int> pos_inf, neg_inf, neut_inf;

    for(int id : idx){
        if((int)pos_inf.size() < num_pos) pos_inf.push_back(id);
        else if((int)neg_inf.size() < num_neg) neg_inf.push_back(id);
        else if((int)neut_inf.size() < num_neutral) neut_inf.push_back(id);
    }

    for(int v : pos_inf)
        nodes[v].opinion = nodes[v].stubbornness = nodes[v].influence = nodes[v].exposure = 1;
    for(int v : neg_inf){
        nodes[v].opinion = -1;
        nodes[v].stubbornness = nodes[v].influence = nodes[v].exposure = 1;
    }
    for(int v : neut_inf){
        nodes[v].opinion = 0;
        nodes[v].stubbornness = nodes[v].influence = nodes[v].exposure = 1;
    }

    // ===== 4. Write edges.csv =====
    {
        ofstream e("edges.csv");
        e << "u,v\n";
        for(int u=0;u<N;u++){
            for(int v: adj[u]) if(u<v)
                e << u << "," << v << "\n";
        }
    }

    // ===== 5. Run Opinion Dynamics =====
    for(int step=0; step<=steps; step++){
        string fname = "nodes_step_" + to_string(step) + ".csv";
        ofstream f(fname);
        f << "id,x,y,opinion,stubbornness,influence,exposure\n";

        for(int i=0;i<N;i++){
            f << i << "," << nodes[i].x << "," << nodes[i].y << ","
              << nodes[i].opinion << "," << nodes[i].stubbornness << ","
              << nodes[i].influence << "," << nodes[i].exposure << "\n";
        }

        if(step == steps) break;

        vector<double> new_op(N);

        for(int u=0; u<N; u++){
            Node &p = nodes[u];
            if(p.stubbornness >= 1){
                new_op[u] = p.opinion;
                continue;
            }

            double ws = 0, wt = 0;
            for(int v : adj[u]){
                if(drand() > p.exposure) continue;
                Node &q = nodes[v];
                ws += q.opinion * q.influence;
                wt += q.influence;
            }

            double neigh = wt>0 ? ws/wt : p.opinion;
            double diff = fabs(neigh - p.opinion);
            double conf = diff <= tolerance ? 1 : 1/(1+(diff-tolerance)*2);

            new_op[u] = p.stubbornness*p.opinion +
                        (1 - p.stubbornness)*(conf*neigh + (1-conf)*p.opinion);

            new_op[u] = max(-1.0, min(1.0, new_op[u]));
        }

        for(int i=0;i<N;i++){
            nodes[i].opinion = new_op[i];
            if(nodes[i].stubbornness < 1)
                nodes[i].stubbornness = min(1.0, nodes[i].stubbornness + stubborn_growth);
        }
    }

    // ====================================================
    // ===== 6. GHETTO DETECTION (SPATIAL + POLARITY) =====
    // ====================================================
    auto sign_group = [&](double op){
        if(fabs(op) < 0.05) return 0;
        return (op > 0) ? 1 : -1;
    };

    DSU dsu(N);

    for(int u=0; u<N; u++){
        for(int v : adj[u]){
            if(u >= v) continue;

            double ou = nodes[u].opinion;
            double ov = nodes[v].opinion;

            bool same_sign = (sign_group(ou) == sign_group(ov));
            bool similar   = fabs(ou - ov) < ghetto_tolerance;

            double dx = nodes[u].x - nodes[v].x;
            double dy = nodes[u].y - nodes[v].y;
            bool spatial_close = (dx*dx + dy*dy < MAX_RADIUS*MAX_RADIUS);

            if(same_sign && similar && spatial_close)
                dsu.unite(u, v);
        }
    }

    // Build clusters
    unordered_map<int, vector<int>> clusters;
    for(int i=0;i<N;i++){
        clusters[dsu.find(i)].push_back(i);
    }

    ofstream out("ghettos.csv");
    out << "ghetto_id,node\n";

    int gid = 0;
    for(auto &p : clusters){
        if(p.second.size() < 5) continue;  
        for(int node : p.second)
            out << gid << "," << node << "\n";
        gid++;
    }

    cout << "Ghetto detection complete. Output -> ghettos.csv\n";

    return 0;
}