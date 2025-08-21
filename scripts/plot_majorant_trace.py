import matplotlib.pyplot as plt
import numpy as np
import sys
import pandas as pd

from matplotlib.ticker import MultipleLocator

dda_trace = pd.read_csv(sys.argv[1])
majorants = pd.read_csv(sys.argv[2])

fig, axs = plt.subplots(2,1)

dda_trace["T"] = dda_trace["T"] * 0.08

ax = axs[0]
ax.plot(dda_trace["T"], dda_trace["Value"], label="Value")
ax.plot(dda_trace["T"], dda_trace["Maximum"], label="Maximum")
ax.plot(dda_trace["T"], np.log2(dda_trace["Dim_getdim"]), label="log2(getDim)", color="purple")
ax.plot(dda_trace["T"], np.log2(dda_trace["Dim_nodeinfo"]), label="log2(nodeinfo.dim)", color="pink")

ax.hlines(majorants["Majorant"], majorants["T0"], majorants["T1"], label="Majorants", color="green")
ax.scatter(pd.concat((majorants["T0"], majorants["T1"])), pd.concat((majorants["Majorant"], majorants["Majorant"])), color="green", s=10)

# ax.grid(which='major', color='black', linestyle='-', linewidth=1.0, alpha=0.7)
# ax.grid(which='minor', color='black', linestyle='-', linewidth=0.5, alpha=0.3)
ax.legend()

ax = axs[1]
ax.scatter(majorants["T0"], majorants["X0"], label="X0", color="orange")
ax.scatter(majorants["T0"], majorants["Y0"], label="Y0", color="lime")
ax.scatter(majorants["T0"], majorants["Z0"], label="Z0", color="cyan")

ax.scatter(majorants["T1"], majorants["X1"], label="X1", color="orange")
ax.scatter(majorants["T1"], majorants["Y1"], label="Y1", color="lime")
ax.scatter(majorants["T1"], majorants["Z1"], label="Z1", color="cyan")

ax.plot(dda_trace["T"], dda_trace["X"], 'o-', label="X", color="red", markersize=3)
ax.plot(dda_trace["T"], dda_trace["Y"], 'o-', label="Y", color="green", markersize=3)
ax.plot(dda_trace["T"], dda_trace["Z"], 'o-', label="Z", color="blue", markersize=3)

ax.legend()


plt.show()