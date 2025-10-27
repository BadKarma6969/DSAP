#include <bits/stdc++.h>
using namespace std;

struct Node {
    double polarity;
};

class Graph {
public:
    int nodes;
    int edges;
    vector<vector<int>> adj;
    vector<Node> nodeLists; // list of graph nodes
    Graph(int n = 0, int e = 0) {
        nodes = n;
        edges = e;
        adj.resize(n);
        nodeLists.resize(n);
        for (int i = 0; i < n; i++)
            nodeList[i].polarity = ((double)rand() / RAND_MAX) * 2.0 - 1.0; //randomly assigning polarity to nodes
    }
    void addEdge(int u, int v) {
        adj[u].push_back(v);
        adj[v].push_back(u);
        edges++;
    }

    int size() const {
        return nodes;
    }
};

//Random Graph being generated using Erdős–Rényi random graph model
Graph randomGraph(int n, double p){
    Graph G(n);
    for(int i = 0; i < n; i++){
        for(int j = i + 1; j < n; j++){
            if (((double)rand() / RAND_MAX) < p) // if a randomly generated no. is lesser than p, an edge is created between them
                // p has to be set manually
                G.addEdge(i, j);
        }
    }
    return G;
}

double randPolarity() {
    return (double(rand()) / RAND_MAX) * 2.0 - 1.0;
}

void polarize(Graph &G, int steps, double influenceStrength = 0.1) {
    int n = G.size();

    for (int step = 0; step < steps; step++) {
        vector<double> newPolarity(n);

        for (int i = 0; i < n; i++) {
            double sum = 0;
            int count = 0;
            for (int nb : G[i].neighbors) {
                double sim = 1.0 - fabs(G[i].polarity - G[nb].polarity); // fabs is just abs for floating point nos.
                sum += G[nb].polarity * sim;  // weighted by similarity
                count++;
            }
            if (count > 0)
                newPolarity[i] = (1 - influenceStrength) * G[i].polarity +
                                  influenceStrength * (sum / count);
            else
                newPolarity[i] = G[i].polarity;
        }

        // update all at once
        for (int i = 0; i < n; i++)
            G[i].polarity = newPolarity[i];
    }
}

void exportGraph(const Graph &G, const string &prefix) {
    ofstream nodes(prefix + "_nodes.csv");
    ofstream edges(prefix + "_edges.csv");

    for (int i = 0; i < G.size(); i++)
        nodes << i << "," << G[i].polarity << "\n";

    for (int i = 0; i < G.size(); i++) {
        for (int nb : G[i].neighbors) {
            if (i < nb)
                edges << i << "," << nb << "\n";
        }
    }

    nodes.close();
    edges.close();
}




int main() {
    int n = 50;
    double p = 0.1;  // has to be set manually
    int steps = 50;

    Graph G = randomGraph(n, p);
    exportGraph(G, "before");

    polarize(G, steps);
    exportGraph(G, "after");

    cout << "Simulation complete. Files saved: before_nodes.csv, before_edges.csv, after_nodes.csv, after_edges.csv\n";
}

