#include <bits/stdc++.h>
using namespace std;
using AdjList = vector<unordered_map<int, double>>;
static std::mt19937_64 rng((unsigned)chrono::high_resolution_clock::now().time_since_epoch().count());

double drand() { return std::uniform_real_distribution<double>(0.0,1.0)(rng); }
int irand(int a, int b) { return std::uniform_int_distribution<int>(a,b)(rng); }

struct Person {
    int id;
    double opinion;       // [-1, +1]
    double stubbornness;  // [0,1] higher -> less change; if ==1.0 then opinion fixed (influencer)
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

    //PARAMETERS
    double agreement_growth = 0.01;    
    double disagreement_decay = 0.02;  
    double min_weight = 0.01;          
    double max_weight = 1.0;
    double tolerance = 0.4;            
    // rewiring disabled in this version (we keep topology fixed)
    double rewiring_prob = 0.0;       

    Graph(int n=0){ init(n); }

    void init(int n){
        N = n;
        adj.assign(n, {});
        persons.clear();
        persons.resize(n);
        for(int i=0;i<n;i++){
            persons[i].id = i;
            
            persons[i].opinion = 0.0;
            persons[i].stubbornness = 0.3 + drand()*0.4; 
            persons[i].influence = 0.4 + drand()*0.6;
            persons[i].exposure = 0.7 + drand()*0.3;
        }
    }

    void add_person(int id, const Person &p){
        if(id < 0 || id >= N) return;
        persons[id] = p;
    }

    void add_edge(int u, int v, double w){
        if(u==v) return;
        adj[u][v] = w;
        adj[v][u] = w;
    }

    void remove_edge(int u, int v){
        adj[u].erase(v);
        adj[v].erase(u);
    }

    void generate_graph(int m0, int m){
        // Barabasi-Albert preferential attachment model 
        if(N<=0) return;
        m0 = max(1, min(m0, N));
        m = max(1, min(m, m0));

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
            unordered_set<int> targets; 
            while((int)targets.size() < m){
                int r = irand(0, total_degree + newv -1);
                int acc = 0;
                for(int i=0; i<newv; i++){
                    acc += degree[i] + 1;
                    if(r < acc){ targets.insert(i); break; }
                }
            }

            for(int v : targets){
                double w = 0.3 + drand()*0.7; 
                add_edge(newv, v, w);
                degree[newv]++; degree[v]++;
                total_degree += 2;
            }
        }
    }

    // seed two influencers - they will be fixed (stubbornness=1.0)
    pair<int,int> seed_influencers(int a=-1, int b=-1){
        if(N < 2) return {-1,-1};
        int i=a, j=b;
        if(i<0 || j<0){
            i = irand(0, N-1);
            j = irand(0, N-1);
            while(j==i) j = irand(0, N-1);
        }
        
        for(int u=0; u<N; ++u){
            persons[u].opinion = 0.0;
            if(persons[u].stubbornness > 0.999) persons[u].stubbornness = 0.9; 
        }

        persons[i].opinion = 1.0; persons[i].stubbornness = 1.0; persons[i].influence = 1.0; persons[i].exposure = 1.0;
        persons[j].opinion = -1.0; persons[j].stubbornness = 1.0; persons[j].influence = 1.0; persons[j].exposure = 1.0;
        return {i,j};
    }

    void update_opinions(){
        vector<double> new_op(N, 0.0);
        for(int u=0; u<N; u++){
            Person &p = persons[u];
            if(p.stubbornness >= 1.0 - 1e-9){

                new_op[u] = p.opinion;
                continue;
            }

            double weighted_sum = 0.0;
            double weight_total = 0.0;
            for(auto &pr : adj[u]){
                int v = pr.first;
                double w = pr.second;

                if(drand() > persons[u].exposure) continue; 

                weighted_sum += w * persons[v].opinion * persons[v].influence;
                weight_total += w * persons[v].influence;
            }

            double neighbor_avg = (weight_total>0.0) ? (weighted_sum / weight_total) : persons[u].opinion;
            
            double diff = fabs(neighbor_avg - persons[u].opinion);
            double confidence_factor = 1.0;
            if(diff > tolerance){
                confidence_factor = 1.0 / (1.0 + (diff - tolerance)*2.0);
            }

            double updated = persons[u].stubbornness * persons[u].opinion + (1.0 - persons[u].stubbornness) * (confidence_factor * neighbor_avg + (1.0 - confidence_factor) * persons[u].opinion);
            
            if(updated > 1.0) updated = 1.0;
            if(updated < -1.0) updated = -1.0;
            new_op[u] = updated;
        }
        for(int i=0;i<N;++i) persons[i].opinion = new_op[i];
    }

    // adjust weights but do NOT remove edges -- keeps graph topology intact
    void modify_weights_no_removal(){
        for(int u=0; u<N; u++){
            for(auto &pr : adj[u]){
                int v = pr.first;
                double &w = pr.second;
                if(u < v){
                    double diff = fabs(persons[u].opinion - persons[v].opinion);
                    if(diff <= tolerance){
                        w += agreement_growth * (1.0 - diff);
                    } else {
                        w -= disagreement_decay * (diff - tolerance);
                    }
                    if(w < min_weight) w = min_weight;
                    if(w > max_weight) w = max_weight;
                    adj[v][u] = w;
                }
            }
        }
    }

    void rewire_edges(){}

    // map opinion [-1,1] to RGB (int 0..255)
    // -1 -> blue (0,0,255); 0 -> white (255,255,255); +1 -> red (255,0,0)
    static tuple<int,int,int> opinion_to_rgb(double op){
        op = max(-1.0, min(1.0, op));
        if(op >= 0.0){
            double t = op; 
            int r = (int)round(255*(1.0 - (1.0 - t))) ; 
            int g = (int)round(255*(1.0 - t));
            int b = (int)round(255*(1.0 - t));
            return {r,g,b};
        } else {
            double t = -op; 
            int r = (int)round(255*(1.0 - t));
            int g = (int)round(255*(1.0 - t));
            int b = 255;
            return {r,g,b};
        }
    }

    void run_simulation(int steps, bool export_csv = true, const string &nodes_file = "nodes.csv", const string &edges_file = "edges.csv"){
        for(int t=0; t<steps; ++t){
            update_opinions();
            modify_weights_no_removal();
        }

        int pos_count = 0, neg_count = 0, neutral_count = 0;
        double threshold = 0.05; 
        for(int i=0;i<N;i++){
            if(persons[i].opinion > threshold) pos_count++;
            else if(persons[i].opinion < -threshold) neg_count++;
            else neutral_count++;
        }

        cout << "Simulation finished after " << steps << " steps.\n";
        cout << "Counts: +" << pos_count << "  -" << neg_count << "  neutral " << neutral_count << "\n";

        if(export_csv){
            ofstream no(nodes_file);
            no << "id,opinion,r,g,b\n";
            for(int i=0;i<N;i++){
                int r, g, b;
                std::tie(r, g, b) = opinion_to_rgb(persons[i].opinion);
                no << i << "," << persons[i].opinion << "," << r << "," << g << "," << b << "\n";

            }
            no.close();
            ofstream eo(edges_file);
            eo << "u,v,weight\n";
            vector<vector<char>> seen(N, vector<char>(N,0));
            for(int u=0; u<N; ++u){
                for(auto &pr : adj[u]){
                    int v = pr.first;
                    double w = pr.second;
                    if(!seen[u][v]){
                        eo << u << "," << v << "," << w << "\n";
                        seen[u][v] = seen[v][u] = 1;
                    }
                }
            }
            eo.close();
            cout << "Exported nodes -> " << nodes_file << " and edges -> " << edges_file << "\n";
            cout << "You can open nodes.csv + edges.csv in Gephi/Cytoscape/d3 for color visualization.\n";
        }

        vector<pair<double,int>> ord;
        for(int i=0;i<N;i++) ord.push_back({persons[i].opinion, i});
        sort(ord.begin(), ord.end(), [](auto &a, auto &b){ return a.first < b.first; });
        cout << "Five most negative nodes (opinion, id):\n";
        for(int k=0;k<min(5, (int)ord.size()); ++k) cout << ord[k].first << " , " << ord[k].second << "\n";
        cout << "Five most positive nodes (opinion, id):\n";
        for(int k=0;k<min(5, (int)ord.size()); ++k) cout << ord[(int)ord.size()-1-k].first << " , " << ord[(int)ord.size()-1-k].second << "\n";
    }

    void print_summary(){
        cout << "Graph N=" << N << " Edges ~ ";
        long long m = 0;
        for(int u=0; u<N; ++u) m += adj[u].size();
        cout << m/2 << "\n";
    }
};


int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int N = 200;         
    int m0 = 5;          
    int m = 3;           
    Graph G(N);
    G.generate_graph(m0, m);


    for(int i=0;i<N;i++){
        G.persons[i].stubbornness = 0.2 + drand()*0.4; 
        G.persons[i].influence = 0.5 + drand()*0.5;    
        G.persons[i].exposure = 0.6 + drand()*0.4;     
        G.persons[i].opinion = 0.0;
    }

    std::pair<int, int> influencers = G.seed_influencers();
    int inf_pos = influencers.first;
    int inf_neg = influencers.second;
    cout << "Seeded influencers: +" << inf_pos << " and -" << inf_neg << "\n";


    G.print_summary();

    int steps = 200;
    G.run_simulation(steps, true, "nodes.csv", "edges.csv");

    return 0;
}
