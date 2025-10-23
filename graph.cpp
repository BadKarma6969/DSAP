#include <bits/stdc++.h>
using namespace std;

struct Node {
    double polarity;
};

class Graph {
  int nodes;
  int edges;
  vector<int> adj(nodes);
  Graph() {nodes = 0; edges = 0; vector<int> adj(0)};
};

Graph G = new Graph();

double randPolarity() {
    return (double(rand()) / RAND_MAX) * 2.0 - 1.0;
}

void polarize(Graph &G, int steps, double influence_strength = 0.1) {
    // int n = G.size();
    int n = G.nodes;

    for (int step = 0; step < steps; step++) {
        vector<double> new_polarity(n);

        for (int i = 0; i < n; i++) {
            double sum = 0;
            int count = 0;
            for (int nb : G[i].neighbors) {
                double sim = 1.0 - fabs(G[i].polarity - G[nb].polarity);
                sum += G[nb].polarity * sim;  // weighted by similarity
                count++;
            }
            if (count > 0)
                new_polarity[i] = (1 - influence_strength) * G[i].polarity +
                                  influence_strength * (sum / count);
            else
                new_polarity[i] = G[i].polarity;
        }

        // update all at once
        for (int i = 0; i < n; i++)
            G[i].polarity = new_polarity[i];
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
    double p = 0.1;  // edge probability
    int steps = 50;

    Graph G = randomGraph(n, p);
    exportGraph(G, "before");

    polarize(G, steps);
    exportGraph(G, "after");

    cout << "Simulation complete. Files saved: before_nodes.csv, before_edges.csv, after_nodes.csv, after_edges.csv\n";
}
