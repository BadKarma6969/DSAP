# Graph Polarization & Community Detection Simulation

## 📌 Project Overview

This project simulates **graph structures** with polarized nodes and detects communities based on node polarities. The main goal is to study how polarization evolves in networks and how communities form over time. It can later be extended to real-world datasets like social media graphs.

**Key features:**

- Generate random graphs with configurable size and connectivity.
- Assign polarities to nodes in the range `[-1, 1]`.
- Simulate community formation using graph structure and node polarities.
- Visualize communities and node polarization.
- Future extensions: diffusion models, real-world graph integration.

---

## 🛠️ Technologies Used

- **Python**: Graph generation, simulation, visualization.
- **NetworkX**: Graph creation and community detection.
- **Matplotlib / Seaborn**: Visualization of nodes and communities.
- **Optional**: C++ modules for performance-critical simulations.

---

## ⚡ Features

### 1. Graph Generation
- Create random graphs with `n` nodes and probability `p` for edge creation.
- Configurable graph density and node attributes.

### 2. Node Polarization
- Assign random polarities in the range `[-1, 1]`.
- Track evolution of node opinions if diffusion is applied later.

### 3. Community Detection
- Detect communities using modularity-based methods.
- Color-code nodes by community for visualization.

### 4. Visualization
- Plot graphs with nodes colored by polarity or community.
- Easily compare polarization vs. community structure.
