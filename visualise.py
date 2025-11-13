import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np
import networkx as nx
from scipy.spatial import ConvexHull
import glob

# Load edges
edges_df = pd.read_csv("edges.csv")
edges = list(zip(edges_df.u, edges_df.v))

# Load all node snapshots
files = sorted(glob.glob("nodes_step_*.csv"),
               key=lambda x: int(x.split("_")[-1].split(".")[0]))

snapshots = [pd.read_csv(f) for f in files]
steps = len(snapshots)

# Build base graph
N = len(snapshots[0])
G = nx.Graph()
G.add_nodes_from(range(N))
G.add_edges_from(edges)

# positions constant
pos = {}
for i, row in snapshots[0].iterrows():
    pos[row.id] = (row.x, row.y)

# Animation
fig, ax = plt.subplots(figsize=(10, 8))
plt.axis('off')

def frame(t):
    ax.clear()
    ax.set_title(f"Opinion Dynamics — Step {t}", fontsize=14)
    ax.axis('off')

    snap = snapshots[t]

    colors = []
    sizes = []
    for i,row in snap.iterrows():
        op = row.opinion
        if op > 0.05:
            colors.append((1,0.4,0.4))
        elif op < -0.05:
            colors.append((0.4,0.4,1))
        else:
            colors.append((0.8,0.8,0.8))
        sizes.append(50 + 300*abs(op))

    nx.draw_networkx_edges(G, pos, alpha=0.1, width=0.4, ax=ax)
    nx.draw_networkx_nodes(G, pos, node_color=colors,
                           node_size=sizes, ax=ax)

anim = animation.FuncAnimation(fig, frame, frames=steps,
                               interval=50, repeat=False)
plt.show()

# Ghettos - on final frame
import matplotlib.pyplot as plt

snap = snapshots[-1]

def sign_group(op):
    if abs(op)<0.05: return 0
    return 1 if op>0 else -1

parent = {i:i for i in range(N)}
def find(x):
    if parent[x]!=x:
        parent[x]=find(parent[x])
    return parent[x]
def union(a,b):
    pa,pb=find(a),find(b)
    if pa!=pb: parent[pb]=pa

# merge similar neighbors
for u,v in edges:
    ou = snap.loc[u].opinion
    ov = snap.loc[v].opinion
    if sign_group(ou)==sign_group(ov) and abs(ou-ov)<0.08:
        union(u,v)

clusters={}
for i in range(N):
    r=find(i)
    clusters.setdefault(r,[]).append(i)

ghettos = [c for c in clusters.values() if len(c)>=5]

plt.figure(figsize=(10,8))
plt.title("Final Ghettos Overlay")

colors=[]
sizes=[]
for i,row in snap.iterrows():
    op=row.opinion
    if op>0.05: colors.append((1,0.4,0.4))
    elif op<-0.05: colors.append((0.4,0.4,1))
    else: colors.append((0.8,0.8,0.8))
    sizes.append(50+300*abs(op))

nx.draw_networkx_edges(G,pos,alpha=0.1)
nx.draw_networkx_nodes(G,pos,node_color=colors,node_size=sizes)

# Draw hulls
for g in ghettos:
    pts=np.array([pos[n] for n in g])
    if len(pts)>=3:
        try:
            hull=ConvexHull(pts)
            hp=pts[hull.vertices]
        except:
            hp=pts
        op = np.mean([snap.loc[n].opinion for n in g])
        if op>0.05: blob=(1,0.3,0.3,0.25)
        elif op<-0.05: blob=(0.3,0.3,1,0.25)
        else: blob=(0.7,0.7,0.7,0.2)
        plt.fill(hp[:,0],hp[:,1],color=blob)

plt.axis('off')
plt.show()
