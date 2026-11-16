# Week 3: plots comparing array tree and pointer tree
import csv
import matplotlib.pyplot as plt

def load(path):
    with open(path) as f:
        rows = list(csv.DictReader(f))
    # tree_d12.h -> 12
    depths = [int(r["tree"].split("_d")[1].split(".")[0]) for r in rows]
    return depths, rows

a_depth, a = load("week3/results/array_results.csv")
p_depth, p = load("week3/results/pointer_results.csv")

def col(rows, name):
    return [float(r[name]) for r in rows]

# 1. Model size vs depth (the main result)
plt.figure()
plt.plot(a_depth, col(a, "size_small_bytes"), "o-", label="Array tree")
plt.plot(p_depth, col(p, "size_small_bytes"), "s-", label="Pointer tree")
plt.yscale("log")
plt.xlabel("Tree depth")
plt.ylabel("Model size (bytes, log scale)")
plt.title("Model size vs depth")
plt.legend()
plt.grid(True)
plt.savefig("week3/results/size_vs_depth.png", dpi=150)

# 2. Empty slots in the array tree
plt.figure()
plt.bar([str(d) for d in a_depth], col(a, "empty_slots"))
plt.xlabel("Tree depth")
plt.ylabel("Empty slots")
plt.title("Array tree: empty slots vs depth")
plt.savefig("week3/results/empty_slots.png", dpi=150)

# 3. Time per prediction
plt.figure()
plt.plot(a_depth, col(a, "ns_per_prediction"), "o-", label="Array tree")
plt.plot(p_depth, col(p, "ns_per_prediction"), "s-", label="Pointer tree")
plt.xlabel("Tree depth")
plt.ylabel("Time per prediction (ns)")
plt.title("Prediction time vs depth")
plt.legend()
plt.grid(True)
plt.savefig("week3/results/time_vs_depth.png", dpi=150)

# 4. Small vs wide fields (array tree)
plt.figure()
x = range(len(a_depth))
plt.bar([i - 0.2 for i in x], col(a, "size_small_bytes"), width=0.4, label="Small fields")
plt.bar([i + 0.2 for i in x], col(a, "size_wide_bytes"), width=0.4, label="32-bit fields")
plt.xticks(list(x), [str(d) for d in a_depth])
plt.yscale("log")
plt.xlabel("Tree depth")
plt.ylabel("Model size (bytes, log scale)")
plt.title("Array tree: small vs 32-bit fields")
plt.legend()
plt.savefig("week3/results/field_size.png", dpi=150)

print("Plots saved in week3/results/")