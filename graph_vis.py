import networkx as nx
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np
import random

try:
    from scipy.spatial import ConvexHull
    have_scipy = True
except ImportError:
    have_scipy = False

# ============================================
# Parameters
# ============================================
N = 400
m = 2
steps = 300
tolerance = 0.25
num_pos = 5
num_neg = 5
num_neutral = 3
edge_keep_ratio = 0.6
degree_tolerance_ratio = 0.1
STUBBORNNESS_GROWTH = 0.003
GHETTO_TOLERANCE = 0.1
VISUAL_EDGE_RADIUS = 0.18   # show only short edges visually

# ============================================
# Helper functions
# ============================================
def drand(): return random.random()

def sign_group(op):
    if abs(op) < 0.05:
        return 0
    return 1 if op > 0 else -1

# ============================================
# Graph creation
# ============================================
G = nx.barabasi_albert_graph(N, m)
edges = list(G.edges())
random.shuffle(edges)
keep = int(len(edges) * edge_keep_ratio)
G.remove_edges_from(edges[keep:])
print(f"Nodes: {G.number_of_nodes()} | Edges after pruning: {G.number_of_edges()}")

for n in G.nodes():
    G.nodes[n]['opinion'] = drand()*0.2 - 0.1
    G.nodes[n]['stubbornness'] = 0.4 + drand()*0.4
    G.nodes[n]['influence'] = 0.5 + drand()*0.5
    G.nodes[n]['exposure'] = 0.7 + drand()*0.3

# ============================================
# Influencer selection — balanced-degree random
# ============================================
degrees = dict(G.degree())
deg_values = np.array(list(degrees.values()))
target_deg = random.choice(deg_values)
band = int(max(1, degree_tolerance_ratio * target_deg))
candidate_nodes = [n for n, d in degrees.items() if abs(d - target_deg) <= band]
if len(candidate_nodes) < num_pos + num_neg + num_neutral:
    band *= 2
    candidate_nodes = [n for n, d in degrees.items() if abs(d - target_deg) <= band]
random.shuffle(candidate_nodes)

pos_influencers = candidate_nodes[:num_pos]
neg_influencers = candidate_nodes[num_pos:num_pos + num_neg]
remaining = [x for x in G.nodes() if x not in pos_influencers + neg_influencers]
neutral_influencers = random.sample(remaining, num_neutral)

for n in pos_influencers:
    G.nodes[n].update({'opinion': 1.0, 'stubbornness': 1.0,
                       'influence': 1.0, 'exposure': 1.0})
for n in neg_influencers:
    G.nodes[n].update({'opinion': -1.0, 'stubbornness': 1.0,
                       'influence': 1.0, 'exposure': 1.0})
for n in neutral_influencers:
    G.nodes[n].update({'opinion': 0.0, 'stubbornness': 1.0,
                       'influence': 1.0, 'exposure': 1.0})

print(f"Influencers +:{pos_influencers}")
print(f"Influencers -:{neg_influencers}")
print(f"Influencers 0:{neutral_influencers}")

# ============================================
# Opinion update
# ============================================
def update_opinions():
    new_op = {}
    for u in G.nodes():
        p = G.nodes[u]
        if p['stubbornness'] >= 1.0 - 1e-9:
            new_op[u] = p['opinion']
            continue
        weighted_sum = 0.0
        weight_total = 0.0
        for v in G.neighbors(u):
            if drand() > p['exposure']:
                continue
            q = G.nodes[v]
            weighted_sum += q['opinion'] * q['influence']
            weight_total += q['influence']
        neighbor_avg = weighted_sum / weight_total if weight_total > 0 else p['opinion']
        diff = abs(neighbor_avg - p['opinion'])
        confidence = 1.0 if diff <= tolerance else 1.0 / (1.0 + (diff - tolerance)*2.0)
        updated = (p['stubbornness'] * p['opinion'] +
                   (1 - p['stubbornness']) *
                   (confidence * neighbor_avg +
                    (1 - confidence) * p['opinion']))
        new_op[u] = np.clip(updated, -1.0, 1.0)
    for u in G.nodes():
        G.nodes[u]['opinion'] = new_op[u]
        if G.nodes[u]['stubbornness'] < 1.0:
            G.nodes[u]['stubbornness'] = min(1.0, G.nodes[u]['stubbornness'] + STUBBORNNESS_GROWTH)

# ============================================
# 2D Layout (spread out)
# ============================================
pos = nx.spring_layout(G, seed=42, k=0.6, iterations=250)
pos = {n: (float(x), float(y)) for n, (x, y) in pos.items()}
x_vals, y_vals = zip(*pos.values())
x_min, x_max = min(x_vals), max(x_vals)
y_min, y_max = min(y_vals), max(y_vals)
pos = {n: ((x - x_min)/(x_max-x_min), (y - y_min)/(y_max-y_min)) for n, (x, y) in pos.items()}
# Add subtle jitter for natural spacing
for n in pos:
    pos[n] = (pos[n][0] + np.random.uniform(-0.01, 0.01),
              pos[n][1] + np.random.uniform(-0.01, 0.01))

# ============================================
# Animation (color = sign, size = opinion magnitude)
# ============================================
fig, ax = plt.subplots(figsize=(10, 8))
plt.axis('off')

def draw_frame(step):
    ax.clear()
    ax.set_title(f"Opinion Dynamics — Step {step}", fontsize=14)
    plt.axis('off')

    node_colors, node_sizes = [], []
    for n in G.nodes():
        op = G.nodes[n]['opinion']
        mag = abs(op)
        # Color by sign
        if op > 0.05:
            color = (1.0, 0.4, 0.4)  # red
        elif op < -0.05:
            color = (0.4, 0.4, 1.0)  # blue
        else:
            color = (0.8, 0.8, 0.8)  # gray
        node_colors.append(color)
        node_sizes.append(50 + 300 * mag)

    local_edges = [(u, v) for (u, v) in G.edges()
                   if np.linalg.norm(np.array(pos[u]) - np.array(pos[v])) < VISUAL_EDGE_RADIUS]

    nx.draw_networkx_edges(G, pos, edgelist=local_edges, alpha=0.15, width=0.8, ax=ax)
    nx.draw_networkx_nodes(G, pos, node_color=node_colors, node_size=node_sizes, ax=ax)

def animate(frame):
    update_opinions()
    draw_frame(frame)

ani = animation.FuncAnimation(fig, animate, frames=steps, interval=100, repeat=False)
plt.show()

# ============================================
# GHETTO DETECTION + OVERLAY BLOBS
# ============================================
print("\nDetecting ghettos...")

parent = {i: i for i in G.nodes()}
def find(x):
    if parent[x] != x:
        parent[x] = find(parent[x])
    return parent[x]
def union(a, b):
    pa, pb = find(a), find(b)
    if pa != pb:
        parent[pb] = pa

# Merge nodes if same ideological side and similar opinion
for u, v in G.edges():
    ou, ov = G.nodes[u]['opinion'], G.nodes[v]['opinion']
    if sign_group(ou) == sign_group(ov) and abs(ou - ov) < GHETTO_TOLERANCE:
        union(u, v)

components = {}
for node in G.nodes():
    root = find(node)
    if root not in components:
        components[root] = []
    components[root].append(node)
ghettos = [g for g in components.values() if len(g) > 5]
print(f"Detected {len(ghettos)} ghettos (≥5 nodes).")

ghetto_avg_opinion = [np.mean([G.nodes[n]['opinion'] for n in g]) for g in ghettos]

# ============================================
# Final Visualization — Ghettos Overlay
# ============================================
plt.figure(figsize=(10, 8))
plt.title("Flat 2D Network — Ghettos Overlay (Node Size ∝ Opinion Strength)", fontsize=14)

node_colors, node_sizes = [], []
for n in G.nodes():
    op = G.nodes[n]['opinion']
    mag = abs(op)
    if op > 0.05:
        color = (1.0, 0.4, 0.4)
    elif op < -0.05:
        color = (0.4, 0.4, 1.0)
    else:
        color = (0.8, 0.8, 0.8)
    node_colors.append(color)
    node_sizes.append(50 + 300 * mag)

nx.draw_networkx_nodes(G, pos, node_color=node_colors, node_size=node_sizes, alpha=0.9)
nx.draw_networkx_edges(G, pos, alpha=0.12, width=0.8)

# Translucent colored blobs (ghettos)
for g, avg_op in zip(ghettos, ghetto_avg_opinion):
    pts = np.array([pos[n] for n in g])
    if len(pts) < 3:
        continue
    try:
        if have_scipy:
            hull = ConvexHull(pts)
            hull_pts = pts[hull.vertices]
        else:
            hull_pts = pts
        # Blob color by sign of avg opinion
        if avg_op > 0.05:
            blob_color = (1.0, 0.4, 0.4, 0.25)
        elif avg_op < -0.05:
            blob_color = (0.4, 0.4, 1.0, 0.25)
        else:
            blob_color = (0.7, 0.7, 0.7, 0.2)
        plt.fill(hull_pts[:, 0], hull_pts[:, 1],
                 color=blob_color, linewidth=0)
    except Exception:
        pass

plt.axis('off')
plt.show()
