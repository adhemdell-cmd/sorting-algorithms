#!/usr/bin/env python3
import csv, os, sys
from collections import defaultdict

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
CSV_PATH = os.path.join(ROOT, "results", "results.csv")
OUT_DIR = os.path.join(ROOT, "results")
TYPES = ["random", "sorted", "reverse", "duplicates"]

data = defaultdict(lambda: defaultdict(dict))
with open(CSV_PATH, newline="") as f:
    for r in csv.DictReader(f):
        data[r["input_type"]][r["algorithm"]][int(r["size"])] = r

def style(name):
    return "--" if "qsort" in name else "-"

fig, axes = plt.subplots(2, 2, figsize=(11, 8), sharex=True)
for ax, t in zip(axes.flat, TYPES):
    for algo, rows in data[t].items():
        sizes = sorted(rows)
        ys = [max(float(rows[s]["wall_ms"]), 1e-4) for s in sizes]
        ax.plot(sizes, ys, style(algo), marker="o", label=algo)
    ax.set_xscale("log"); ax.set_yscale("log")
    ax.set_title(f"{t} data"); ax.grid(True, which="both", alpha=.3)
    ax.set_xlabel("array size n"); ax.set_ylabel("time (ms)")
axes[0][0].legend(fontsize=8)
fig.suptitle("Execution time vs array size (log-log)")
fig.tight_layout(); fig.savefig(os.path.join(OUT_DIR, "time_vs_size.png"), dpi=130)

n = 100000
fig, ax = plt.subplots(figsize=(11, 5))
algos = [a for a in data["random"] if "qsort" not in a]
w = 0.2
for i, t in enumerate(TYPES):
    ys = [max(float(data[t][a][n]["wall_ms"]), 1e-3) for a in algos]
    ax.bar([x + i * w for x in range(len(algos))], ys, w, label=t)
ax.set_xticks([x + 1.5 * w for x in range(len(algos))]); ax.set_xticklabels(algos)
ax.set_yscale("log"); ax.set_ylabel("time (ms, log scale)")
ax.set_title(f"Time for n = {n:,} by input type"); ax.legend(); ax.grid(True, axis="y", alpha=.3)
fig.tight_layout(); fig.savefig(os.path.join(OUT_DIR, "time_n100000_by_type.png"), dpi=130)

fig, ax = plt.subplots(figsize=(7, 4))
for algo in algos:
    sizes = sorted(data["random"][algo])
    ax.plot(sizes, [float(data["random"][algo][s]["memory_mb"]) for s in sizes], marker="o", label=algo)
ax.set_xscale("log"); ax.set_xlabel("array size n"); ax.set_ylabel("memory used by the sort (MB)")
ax.set_title("Auxiliary memory vs size (random data)"); ax.legend(); ax.grid(True, alpha=.3)
fig.tight_layout(); fig.savefig(os.path.join(OUT_DIR, "memory_vs_size.png"), dpi=130)
print("Charts written to", OUT_DIR)
