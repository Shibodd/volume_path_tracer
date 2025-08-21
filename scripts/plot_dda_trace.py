import matplotlib.pyplot as plt
import numpy as np
import sys
import pandas as pd

from matplotlib.ticker import MultipleLocator

data = pd.read_csv(sys.argv[1])

fig, ax = plt.subplots()

ax.plot(data["T"], data["Value"], 'o-', markersize=3, label="Value")
ax.plot(data["T"], data["Maximum"], label="Maximum")
ax.plot(data["T"], np.log2(data["Dim_getdim"]), label="Dim_getdim")
ax.plot(data["T"], np.log2(data["Dim_nodeinfo"]), label="Dim_nodeinfo")
ax.plot(data["T"], data["Active"], label="Active")

ax.xaxis.set_major_locator(MultipleLocator(128))
ax.xaxis.set_minor_locator(MultipleLocator(8))

ax.grid(which='major', color='black', linestyle='-', linewidth=1.0, alpha=0.7)
ax.grid(which='minor', color='black', linestyle='-', linewidth=0.5, alpha=0.3)
ax.legend()

plt.show()