import glob
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation

files = sorted(glob.glob("snaps/snap_*.txt"))

frames =[]
for f in files:
    frames.append(np.loadtxt(f))

fig, ax = plt.subplots()
im = ax.imshow(frames[0], cmap="inferno", vmin=0, vmax=100)

def update(k):
    im.set_data(frames[k])
    ax.set_title(f"frame {k}")
    return [im]

anim = FuncAnimation(fig, update, frames=len(frames), interval=50)
anim.save("heat.gif", writer="pillow")