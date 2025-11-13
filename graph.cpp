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

int main() {
    int N = 1000;
    int m = 2;
    int steps = 300;

    double tolerance = 0.25;
    double stubborn_growth = 0.003;
    double ghetto_tolerance = 0.1;

    double MAX_RADIUS = 0.12;

    int num_pos = 20;
    int num_neg = 20;
    int num_neutral = 0;

    vector<Node> nodes(N);
    vector<vector<int>> adj(N);

    // ===== 1. Create nodes with random positions =====
    for(int i=0;i<N;i++){
        nodes[i].x = drand();
        nodes[i].y = drand();
        nodes[i].opinion = drand()*0.2 - 0.1;
        nodes[i].stubbornness = 0.4 + drand()*0.4;
        nodes[i].influence = 0.5 + drand()*0.5;
        nodes[i].exposure = 0.7 + drand()*0.3;
    }

    // ===== 2. Build Spatial Barabási–Albert =====
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

        // fallback if no candidates
        if(candidates.empty()){
            vector<pair<double,int>> dist_all;
            for(int j=0;j<i;j++){
                double dx = nodes[i].x - nodes[j].x;
                double dy = nodes[i].y - nodes[j].y;
                double dist = sqrt(dx*dx + dy*dy);
                dist_all.push_back({dist,j});
            }
            sort(dist_all.begin(), dist_all.end());
            for(int k=0;k<min(20,(int)dist_all.size());k++)
                candidates.push_back({1, dist_all[k].second});
        }

        // pick top m by degree
        sort(candidates.begin(), candidates.end(),
            [&](auto &a, auto &b){ return a.first > b.first; });
        for(int k=0;k<m && k<candidates.size();k++){
            int v = candidates[k].second;
            adj[i].push_back(v);
            adj[v].push_back(i);
        }
    }

    // ===== 3. Select influencers =====
    vector<int> degrees(N);
    for(int i=0;i<N;i++) degrees[i] = adj[i].size();
    vector<int> idx(N);
    iota(idx.begin(), idx.end(), 0);
    shuffle(idx.begin(), idx.end(), rng);

    vector<int> pos_inf, neg_inf, neut_inf;

    for(int id: idx){
        if(pos_inf.size() < num_pos) pos_inf.push_back(id);
        else if(neg_inf.size() < num_neg) neg_inf.push_back(id);
        else if(neut_inf.size() < num_neutral) neut_inf.push_back(id);
    }

    for(int v: pos_inf)
        nodes[v].opinion = nodes[v].stubbornness = nodes[v].influence = nodes[v].exposure = 1;
    for(int v: neg_inf)
        nodes[v].opinion = -1, nodes[v].stubbornness = nodes[v].influence = nodes[v].exposure = 1;
    for(int v: neut_inf)
        nodes[v].opinion = nodes[v].stubbornness = nodes[v].influence = nodes[v].exposure = 1;

    // ===== 4. Write static edges.csv =====
    {
        ofstream e("edges.csv");
        e << "u,v\n";
        for(int u=0;u<N;u++){
            for(int v: adj[u]){
                if(u < v) e << u << "," << v << "\n";
            }
        }
    }

    // ===== 5. Run dynamics & write nodes_step_t.csv =====
    for(int step=0; step<=steps; step++){
        // save snapshot
        {
            string fname = "nodes_step_" + to_string(step) + ".csv";
            ofstream f(fname);
            f << "id,x,y,opinion,stubbornness,influence,exposure\n";
            for(int i=0;i<N;i++){
                f << i << "," << nodes[i].x << "," << nodes[i].y << ","
                  << nodes[i].opinion << "," << nodes[i].stubbornness << ","
                  << nodes[i].influence << "," << nodes[i].exposure << "\n";
            }
        }

        if(step == steps) break;

        // update
        vector<double> new_op(N);
        for(int u=0;u<N;u++){
            Node &p = nodes[u];
            if(p.stubbornness >= 1){
                new_op[u] = p.opinion;
                continue;
            }
            double ws=0, wt=0;
            for(int v: adj[u]){
                if(drand() > p.exposure) continue;
                Node &q = nodes[v];
                ws += q.opinion * q.influence;
                wt += q.influence;
            }
            double neigh = wt>0 ? ws/wt : p.opinion;
            double diff = fabs(neigh - p.opinion);
            double conf = diff<=tolerance ? 1 : 1/(1+(diff-tolerance)*2);
            new_op[u] = p.stubbornness*p.opinion +
                        (1-p.stubbornness)*(conf*neigh + (1-conf)*p.opinion);
            new_op[u] = max(-1.0, min(1.0, new_op[u]));
        }

        for(int i=0;i<N;i++){
            nodes[i].opinion = new_op[i];
            if(nodes[i].stubbornness < 1)
                nodes[i].stubbornness = min(1.0, nodes[i].stubbornness + stubborn_growth);
        }
    }

    return 0;
}

