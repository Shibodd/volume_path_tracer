import matplotlib.pyplot as plt
import numpy as np
import sys
import pandas as pd

from matplotlib.ticker import MultipleLocator

dda_trace = pd.read_csv(sys.argv[1])
majorants = pd.read_csv(sys.argv[2])

fig, ax = plt.subplots()

ax.plot(dda_trace["X"], dda_trace["Value"], label="Value")
ax.plot(dda_trace["X"], dda_trace["Maximum"], label="Maximum")
ax.plot(dda_trace["X"], np.log2(dda_trace["Dim_getdim"]), label="log2(getDim)", color="purple")

ax.hlines(majorants["Majorant"], majorants["X0"], majorants["X1"], label="Majorants", color="green")
ax.scatter(pd.concat((majorants["X0"], majorants["X1"])), pd.concat((majorants["Majorant"], majorants["Majorant"])), color="green", s=10)

ax.xaxis.set_major_locator(MultipleLocator(128))
ax.xaxis.set_minor_locator(MultipleLocator(8))

ax.grid(which='major', color='black', linestyle='-', linewidth=1.0, alpha=0.7)
ax.grid(which='minor', color='black', linestyle='-', linewidth=0.5, alpha=0.3)
ax.legend()

plt.show()