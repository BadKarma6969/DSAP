import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np
import networkx as nx
from scipy.spatial import ConvexHull
import glob

# ============================================
# LOAD GRAPH AND SNAPSHOTS
# ============================================

# Load edges
edges_df = pd.read_csv("edges.csv")
edges = list(zip(edges_df.u, edges_df.v))

# Load all snapshot CSV files
files = sorted(
    glob.glob("nodes_step_*.csv"),
    key=lambda x: int(x.split("_")[-1].split(".")[0])
)

snapshots = [pd.read_csv(f) for f in files]
steps = len(snapshots)

# Build static graph
N = len(snapshots[0])
G = nx.Graph()
G.add_nodes_from(range(N))
G.add_edges_from(edges)

# Extract fixed node positions from step 0
pos = {}
for _, row in snapshots[0].iterrows():
    pos[row.id] = (row.x, row.y)

# ============================================
# ANIMATION (Opinion Dynamics)
# ============================================

fig, ax = plt.subplots(figsize=(10, 8))
plt.axis('off')

def frame(t):
    ax.clear()
    ax.set_title(f"Opinion Dynamics — Step {t}", fontsize=14)
    ax.axis('off')

    snap = snapshots[t]
    colors = []
    sizes = []

    for _, row in snap.iterrows():
        op = row.opinion

        # Node color by polarity
        if op > 0.05:
            colors.append((1, 0.4, 0.4))   # red
        elif op < -0.05:
            colors.append((0.4, 0.4, 1))   # blue
        else:
            colors.append((0.8, 0.8, 0.8)) # neutral gray

        # Bubble size = |opinion|
        sizes.append(50 + 300 * abs(op))

    nx.draw_networkx_edges(G, pos, alpha=0.1, width=0.4, ax=ax)
    nx.draw_networkx_nodes(G, pos, node_color=colors, node_size=sizes, ax=ax)

ani = animation.FuncAnimation(
    fig, frame, frames=steps, interval=50, repeat=False
)

plt.show()

# ============================================
# LOAD GHETTOS
# ============================================

ghetto_df = pd.read_csv("ghettos.csv")   # ghetto_id, node
final_snap = snapshots[-1]

# Build dictionary: ghetto_id -> list of nodes
ghettos = {}
for _, row in ghetto_df.iterrows():
    gid = row.ghetto_id
    ghettos.setdefault(gid, []).append(row.node)

# ============================================
# FINAL FRAME: SPATIAL GHETTO OVERLAY
# ============================================

plt.figure(figsize=(10, 8))
plt.title("Final Ghettos Overlay (Convex Hulls)")

colors = []
sizes = []

for _, row in final_snap.iterrows():
    op = row.opinion

    if op > 0.05:
        colors.append((1, 0.4, 0.4))
    elif op < -0.05:
        colors.append((0.4, 0.4, 1))
    else:
        colors.append((0.8, 0.8, 0.8))

    sizes.append(50 + 300*abs(op))

nx.draw_networkx_edges(G, pos, alpha=0.1)
nx.draw_networkx_nodes(G, pos, node_color=colors, node_size=sizes)

# Draw convex hull for each ghetto
for gid, nodes_list in ghettos.items():
    pts = np.array([pos[n] for n in nodes_list])

    if len(pts) < 3:
        continue

    try:
        hull = ConvexHull(pts)
        hull_pts = pts[hull.vertices]
    except:
        hull_pts = pts

    avg_opinion = np.mean([final_snap.loc[n].opinion for n in nodes_list])

    # Color hull based on average polarity
    if avg_opinion > 0.05:
        blob = (1, 0.3, 0.3, 0.25)
    elif avg_opinion < -0.05:
        blob = (0.3, 0.3, 1, 0.25)
    else:
        blob = (0.7, 0.7, 0.7, 0.25)

    plt.fill(hull_pts[:, 0], hull_pts[:, 1], color=blob)

plt.axis('off')
plt.show()