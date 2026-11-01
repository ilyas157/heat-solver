import numpy as np
import matplotlib.pyplot as plt

T = np.loadtxt("mpi_data.txt")
print(T.shape)


plt.imshow(T, cmap="inferno")
plt.colorbar()
plt.savefig("heat.png")
plt.show()
