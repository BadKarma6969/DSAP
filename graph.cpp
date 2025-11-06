#include <bits/stdc++.h>
using namespace std;
using AdjList = vector<unordered_map<int, double>>;
static std::mt19937_64 rng((unsigned)chrono::high_resolution_clock::now().time_since_epoch().count());

double drand() { return std::uniform_real_distribution<double>(0.0,1.0)(rng); }
int irand(int a, int b) { return std::uniform_int_distribution<int>(a,b)(rng); }

struct Person {
    int id;
    double opinion;       // [-1, +1]
    double stubbornness;  // [0,1] higher -> less change
    double influence;     // [0,1] multiplier for other's opinions
    double exposure;      // [0,1] probability to be exposed to neighbor each timestep
    Person() = default;
    Person(int id_, double op, double st, double inf, double exp)
        : id(id_), opinion(op), stubbornness(st), influence(inf), exposure(exp) {}
};

class Graph {
public:
    int N;
    AdjList adj;
    vector<Person> persons;

    //PARAMS
    double agreement_growth = 0.01;    // how much to increase weight on agreement
    double disagreement_decay = 0.02;  // how much to decrease weight on disagreement
    double min_weight = 0.01;          // below this, edge is removed
    double max_weight = 1.0;
    double tolerance = 0.4;            // bounded confidence tolerance for trusting
    double rewiring_prob = 0.05;       // prob to create new edge to random similar node
    
    Graph(int n=0){init(n);}

    void init(int n){
        N = n;
        adj.assign(n, {});
        persons.clear();
        persons.resize(n);
    }

    void add_person(int id, const Person &p){
        if(id < 0 || id >= N) return;
        persons[id] = p;
    }

    void add_edge(int u, int v, double w){
        if(u==v || w==0) return;
        adj[u][v] = w;
        adj[v][u] = w;
    }

    void remove_edge(int u, int v){
        adj[u].erase(v);
        adj[v].erase(u);
    }

    void generate_graph(int m0, int m){
        // m0 : number of initial nodes, that will be fully connected
        // m : number of edges to attach from a new node to existing nodes.
        // Barabasi-Albert preferential attachment model
        init(N);
        if(N<=0) return;
        m0 = max(1, min(m0, N));
        m = max(1, min(m, m0));

        // fully connect initial m0 nodes
        for(int i=0; i<m0; i++){
            for(int j=i+1; j<m0; j++){
                add_edge(i, j, 0.5 + drand()*0.5);
            }
        }
    
        vector<int> degree(N, 0);
        for(int i=0; i<m0; i++){
            degree[i] = (int)adj[i].size();
        }
        int total_degree = accumulate(degree.begin(), degree.begin()+m0, 0);
        for(int newv = m0; newv < N; newv++){
            unordered_set<int> targets; // targets that the new node will connect to
            while((int)targets.size() < m){
                int pick = irand(0, max(newv-1,0));
                int r = irand(0, total_degree + newv -1);
                int acc = 0;
                for(int i=0; i<newv; i++){
                    acc += degree[i] + 1;
                    if(r < acc){ targets.insert(i); break; }
                }
            }

            for(int v : targets){
                double w = 0.3 + drand()*0.7; // random weight between 0.3 and 1.0
                add_edge(newv, v, w);
                degree[newv]++; degree[v]++;
                total_degree += 2;
            }
        }
    }

    void update_opinions(){
        vector<double> new_op(N, 0.0);
        for(int u=0; u<N; u++){
            Person &p = persons[u];
            double weighted_sum = 0.0;
            double weight_total = 0.0;
            for(auto &pr : adj[u]){
                int v = pr.first;
                double w = pr.second;

                if(drand() > persons[u].exposure) continue; // sometimes the person won't get exposed to this neighbour's opinions

                weighted_sum += w * persons[v].opinion * persons[v].influence;
                weight_total += w;
            }

            double neighbor_avg = (weight_total>0.0) ? (weighted_sum / weight_total) : persons[u].opinion;
            new_op[u] = persons[u].stubbornness * persons[u].opinion + (1.0 - persons[u].stubbornness) * neighbor_avg;

            // clamp to [-1, +1]
            if(new_op[u] > 1.0) new_op[u] = 1.0;
            if(new_op[u] < -1.0) new_op[u] = -1.0;
        }
        for(int i=0;i<N;++i) persons[i].opinion = new_op[i];
    }

    void modify_weights(){
        vector<pair<int,int>> to_remove;
        for(int u=0; u<N; u++){
            for(auto &pr : adj[u]){
                int v = pr.first;
                double &w = pr.second;
                if(u<v){
                    double diff = fabs(persons[u].opinion - persons[v].opinion);
                    if(diff <= tolerance){
                        w += agreement_growth * (1.0 - diff);
                        if(w > max_weight) w = max_weight;
                    } else {
                        w -= disagreement_decay * (diff - tolerance);
                        if(w < min_weight){
                            to_remove.push_back({u,v});
                        }
                        else{
                            for(auto &pr2 : adj[v]) if(pr2.first==u) { pr2.second = w; break; }
                        }
                    }
                }
            }
        }
        for(auto &pr : to_remove){
            remove_edge(pr.first, pr.second);
        }
    }

    void rewire_edges(){
        for(int u=0; u<N; u++){
            if(drand() < rewiring_prob){
                int candidate = irand(0, N-1);
                if(candidate != u && adj[u].find(candidate) == adj[u].end()){ // ensure that it is not already connected
                    double op_diff = fabs(persons[u].opinion - persons[candidate].opinion);
                    if(op_diff <= tolerance){
                        double w = 0.1 + drand()*0.9;
                        add_edge(u, candidate, w);
                    }
                }
            }
        }
    }

    //Todo: Add metrics, simulation functions, etc.
};
