#include <bits/stdc++.h>
using namespace std;

// chrono::steady_clock::now().time_since_epoch().count() for dynamic seed

static mt19937_64 rng(6942069);
double drand() { return uniform_real_distribution<double>(0.0,1.0)(rng); }

struct Node {
    double x, y;
    double opinion; //[-1, 1]
    double stubbornness; //How much a node is willing to listen to some other node. [0, 1]
    double influence; //How strongly a node can influence others. [0, 1]
};

struct KDNode {
    int idx;
    double x, y;
    KDNode *left, *right;
    
    KDNode(int i, double _x, double _y) : idx(i), x(_x), y(_y), left(nullptr), right(nullptr) {}
};

class KDTree {
private:
    KDNode* root;
    
    KDNode* buildTree(vector<pair<pair<double,double>,int>>& points, int depth, int start, int end) {
        if(start >= end) return nullptr;
        
        int axis = depth % 2;
        int mid = (start + end) / 2;
        
        // Sort by x or y based on axis
        nth_element(points.begin() + start, points.begin() + mid, points.begin() + end,
            [axis](const auto& a, const auto& b) {
                return axis == 0 ? a.first.first < b.first.first : a.first.second < b.first.second;
            });
        
        KDNode* node = new KDNode(points[mid].second, points[mid].first.first, points[mid].first.second);
        node->left = buildTree(points, depth + 1, start, mid);
        node->right = buildTree(points, depth + 1, mid + 1, end);
        
        return node;
    }
    
    void rangeSearch(KDNode* node, double qx, double qy, double radius, 
                    vector<int>& result, int depth) {
        if(!node) return;
        
        double dx = node->x - qx;
        double dy = node->y - qy;
        double dist_sq = dx*dx + dy*dy;
        
        if(dist_sq <= radius * radius) {
            result.push_back(node->idx);
        }
        
        int axis = depth % 2;
        double diff = axis == 0 ? (qx - node->x) : (qy - node->y);
        
        // Search the near side first
        if(diff < 0) {
            rangeSearch(node->left, qx, qy, radius, result, depth + 1);
            if(diff * diff <= radius * radius) {
                rangeSearch(node->right, qx, qy, radius, result, depth + 1);
            }
        } else {
            rangeSearch(node->right, qx, qy, radius, result, depth + 1);
            if(diff * diff <= radius * radius) {
                rangeSearch(node->left, qx, qy, radius, result, depth + 1);
            }
        }
    }
    
public:
    KDTree() : root(nullptr) {}
    
    void build(vector<Node>& nodes, int n) {
        vector<pair<pair<double,double>,int>> points;
        points.reserve(n);
        for(int i = 0; i < n; i++) {
            points.push_back({{nodes[i].x, nodes[i].y}, i});
        }
        root = buildTree(points, 0, 0, n);
    }
    
    vector<int> queryRange(double x, double y, double radius) {
        vector<int> result;
        rangeSearch(root, x, y, radius, result, 0);
        return result;
    }
};

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
    
    //Create node positions (x,y) and attributes
    for(int i=0;i<N;i++){
        nodes[i].x = drand();
        nodes[i].y = drand();
        nodes[i].opinion = drand()*0.2 - 0.1;
        nodes[i].stubbornness = 0.4 + drand()*0.4;
        nodes[i].influence = 0.5 + drand()*0.5;
    }
    
    //Graph generation using Barbarasi Albert Model, using Kd-Tree to optimize spatial preference
    for(int i=1;i<N;i++){
        KDTree kdtree;
        kdtree.build(nodes, i);
        
        vector<int> nearby = kdtree.queryRange(nodes[i].x, nodes[i].y, MAX_RADIUS);
        vector<pair<double,int>> candidates;
        
        for(int j : nearby) {
            int deg = adj[j].size()+1;
            candidates.push_back({deg, j});
        }
        
        if(candidates.empty()){
            // Fall back to closest nodes
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
    
    //Select initial N opionionated nodes
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
        nodes[v].opinion = nodes[v].stubbornness = nodes[v].influence = 1;
    for(int v : neg_inf){
        nodes[v].opinion = -1;
        nodes[v].stubbornness = nodes[v].influence = 1;
    }
    for(int v : neut_inf){
        nodes[v].opinion = 0;
        nodes[v].stubbornness = nodes[v].influence = 1;
    }
    
    //Export to csv
    {
        ofstream e("edges.csv");
        e << "u,v\n";
        for(int u=0;u<N;u++){
            for(int v: adj[u]) if(u<v)
                e << u << "," << v << "\n";
        }
    }
    
    //Simulation of opinion spread
    vector<double> new_op(N);
    vector<bool> converged(N, false);
    
    for(int step=0; step<=steps; step++){
        string fname = "nodes_step_" + to_string(step) + ".csv";
        ofstream f(fname);
        f << "id,x,y,opinion,stubbornness,influence\n";
        
        // Buffered output
        string buffer;
        buffer.reserve(N * 80);
        
        for(int i=0;i<N;i++){
            buffer += to_string(i) + "," + to_string(nodes[i].x) + "," + 
                      to_string(nodes[i].y) + "," + to_string(nodes[i].opinion) + "," + 
                      to_string(nodes[i].stubbornness) + "," + 
                      to_string(nodes[i].influence) + "\n";
        }
        f << buffer;
        
        if(step == steps) break;

        #pragma omp parallel for
        for(int u=0; u<N; u++){
            Node &p = nodes[u];
            if(converged[u] || p.stubbornness >= 1){
                new_op[u] = p.opinion;
                continue;
            }
            
            double ws = 0, wt = 0;
            for(int v : adj[u]){
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
            
            //Check convergence
            if(fabs(new_op[u] - p.opinion) < 1e-6)
                converged[u] = true;
        }
        
        for(int i=0;i<N;i++){
            nodes[i].opinion = new_op[i];
            if(nodes[i].stubbornness < 1)
                nodes[i].stubbornness = min(1.0, nodes[i].stubbornness + stubborn_growth);
        }
    }
    
    //Assigning ghettos using Union-Find
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
    
    //Build clusters
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
    
    cout << "Graph generation and simulation complete. Run visualise.py to visualise the graph.";
    
    return 0;
}