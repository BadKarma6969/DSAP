#include <bits/stdc++.h>
using namespace std;
using AdjList = vector<unordered_map<int, double>>;

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

    //Todo: Add functions for graph generation, opinion updates, etc.
};
