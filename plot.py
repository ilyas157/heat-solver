import numpy as np
import matplotlib.pyplot as plt
import sys


file = sys.argv[1] if len(sys.argv) >1 else "data.txt"
T = np.loadtxt(file)
print(T.shape)


plt.imshow(T, cmap="inferno")
plt.colorbar()
plt.savefig("heat.png")
plt.show()
