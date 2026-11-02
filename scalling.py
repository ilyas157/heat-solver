import re
import sys
import matplotlib.pyplot as plt

# file name from the command line
fname = sys.argv[1] if len(sys.argv) > 1 else "scaling.out"

# read all (ranks, time) pairs
best = {}            # ranks -> best time
ranks = None
with open(fname) as f:
    for line in f:
        m = re.search(r"ranks=(\d+)", line)
        if m:
            ranks = int(m.group(1))
        m = re.search(r"timing= ([\d.]+)", line)
        if m and ranks is not None:
            t = float(m.group(1))
            if ranks not in best or t < best[ranks]:
                best[ranks] = t

# compute speedup and efficiency
P = sorted(best)
T = [best[p] for p in P]
S = [T[0] / t for t in T]
E = [s / p for s, p in zip(S, P)]

# save CSV
with open("scaling.csv", "w") as f:
    f.write("ranks,time,speedup,efficiency\n")
    for p, t, s, e in zip(P, T, S, E):
        f.write(f"{p},{t:.4f},{s:.3f},{e:.3f}\n")
        print(f"{p:3d} ranks  {t:7.3f} s  speedup {s:5.2f}  efficiency {e:5.1%}")

# speedup plot
plt.figure()
plt.plot(P, S, "o-", label="measured")
plt.plot(P, P, "--", label="ideal")
plt.xscale("log", base=2)
plt.yscale("log", base=2)
plt.xticks(P, P)
plt.yticks(P, P)
plt.xlabel("MPI ranks")
plt.ylabel("speedup")
plt.title("Strong scaling, N=4000, 1000 iterations (FT3)")
plt.legend()
plt.grid(True, alpha=0.3)
plt.savefig("speedup.png", dpi=150)

# efficiency plot
plt.figure()
plt.plot(P, [e * 100 for e in E], "o-")
plt.xscale("log", base=2)
plt.xticks(P, P)
plt.ylim(0, 105)
plt.xlabel("MPI ranks")
plt.ylabel("efficiency (%)")
plt.title("Parallel efficiency, N=4000, 1000 iterations (FT3)")
plt.grid(True, alpha=0.3)
plt.savefig("efficiency.png", dpi=150)
