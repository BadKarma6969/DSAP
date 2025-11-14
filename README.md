# Modeling the spread of  ideology in a population

##  Project Overview

This project simulates **graph structures** with some idealogically polarized nodes and detects communities based on node polarities. The main goal is to study how polarization evolves in networks and how communities form over time. Graph based approaches for modeling idealogical belifs in communities is an incresingly used tool in political and behavioural sciences (Socigrams).
**Key features:**

- Generate random graphs with configurable size and connectivity.
- Assign polarities to nodes in the range `[-1, 1]`.
- Simulate idealogical spread using graph structure and node polarities.
- Detects communities of similar ideaologies 
- Visualize communities and node polarization.

---

##  Technologies Used

-  **Python: NetworkX MatPlotLib**: Only for graph visualisation
-  **C++**: Graph creation and community detection.

---

##  Features

### 1. Graph Generation
- Create random graphs with `n` nodes and probability `p` for edge creation.
- Configurable graph density and node attributes.

### 2. Node Polarization
- Assign random polarities in the range `[-1, 1]`.
- Track evolution of node opinions if diffusion is applied later.

### 3. Community Detection
- Detect communities using Union Find

### 4. Visualization
- Plot graphs with nodes colored by polarity or community.
- Easily compare polarization vs. community structure.
