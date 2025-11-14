# Modeling the spread of  ideology in a population

##  Project Overview

This project simulates a graph with some idealogically polarized nodes and detects communities based on node polarities. The main goal is to study how polarization evolves in networks and how communities of similar opinons form over time. Graph based approaches for modeling idealogical beliefs in communities is an incresingly used tool in political and behavioural sciences (Sociograms).

**Key features:**

- Generates random graphs.
- Assigns polarities to nodes in the range `[-1, 1]`.
- Simulate idealogical spread using graph structure and neighbouring node polarities.
- Detects communities of similar opinions(ideaology) using union find 
- Visualize communities(idealogical ghettos) and node polarization.

---

##  Technologies Used
-  **C++**: Graph creation, simulation and community detection.
-  **Python:** NetworkX, MatPlotLib (for graph visualisation)


---

##  Features

### 1. Graph Generation

- Builds a random graph using the **spatial Barabási–Albert Model**.
- A **KD-Tree** enables fast spatial neighbor searches.


### 2. Node Polarization

- Each node is assigned:
  - **Opinion** - numerical representation of a node's opinion
  - **Stubbornness** — resistance to influence  
  - **Influence** — strength of persuasion

- Opinion updates use:
  - **Weighted average of one's own and neighbouring opinions**
  - **Bounded-confidence filtering** (ignoring highly dissimilar neighbors)
  - **Increasing Stubbornness**, gradually hardening beliefs



### 3. Community Detection

- Uses **Disjoint Set Union (Union–Find)** to detect polarized spatial clusters (ghettos).
- Communities form when nodes meet all criteria:
  - **Spatial proximity**
  - **Ideological similarity**

- DSU aggregates these into distinct ideological communities.

### 4. Visualization
- Plot graphs with nodes colored by opinion 
- Plot overlay of community in the graph
- Easily compare polarization vs. community structure.

![humara graph](https://github.com/anjaneya-damle/anjaneya-damle.github.io/blob/main/ezgif-8bd1a83a157b8a09.gif)
