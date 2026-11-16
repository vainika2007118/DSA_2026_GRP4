"""train_tree.py - trains a decision tree on features.csv and writes it out.

Usage:  python train_tree.py 5        (5 = maximum depth)
Needs:  pip install scikit-learn numpy

It prints the tree as a table and writes tree_d<depth>.h for your C code.
Each row: node, feature (-1 for a leaf), threshold, left, right, class.
Rule at an inner node:  if (x[feature] <= threshold) go left, else go right.
"""
import sys, csv
import numpy as np
from sklearn.tree import DecisionTreeClassifier

depth = int(sys.argv[1]) if len(sys.argv) > 1 else 5
rows = list(csv.reader(open('features.csv')))
header, data = rows[0], np.array(rows[1:], dtype=int)
X, y = data[:, :-1], data[:, -1]
n_train = int(len(X) * 0.7)            # first 70% train, last 30% test
clf = DecisionTreeClassifier(max_depth=depth, random_state=0).fit(X[:n_train], y[:n_train])
t = clf.tree_

nodes = []
for i in range(t.node_count):
    if t.children_left[i] == -1:
        nodes.append((i, -1, 0, -1, -1, int(np.argmax(t.value[i][0]))))
    else:
        # features are integers, so "x <= 120.5" is the same as "x <= 120"
        nodes.append((i, int(t.feature[i]), int(np.floor(t.threshold[i])),
                      int(t.children_left[i]), int(t.children_right[i]), -1))

def predict(x):
    i = 0
    while nodes[i][1] != -1:
        i = nodes[i][3] if x[nodes[i][1]] <= nodes[i][2] else nodes[i][4]
    return nodes[i][5]

pred = np.array([predict(x) for x in X])
assert (pred == clf.predict(X)).all()
acc = (pred[n_train:] == y[n_train:]).mean() * 100
print(f'depth {depth}: {len(nodes)} nodes, actual depth {clf.get_depth()}, test accuracy {acc:.1f}%')
print('node feature threshold left right class')
for n in nodes:
    print(*n)

name = f'tree_d{depth}.h'
with open(name, 'w') as f:
    g = f'TREE_D{depth}_H'
    f.write(f'/* {name} - decision tree, max depth {depth}, made by train_tree.py\n')
    f.write(f' * {len(nodes)} nodes, test accuracy {acc:.1f}%. Leaf: feature = -1, class = 0 healthy / 1 faulty.\n */\n')
    f.write(f'#ifndef {g}\n#define {g}\n\n#include <stdint.h>\n\n')
    f.write('#ifndef TREE_ROW_T_DEFINED\n#define TREE_ROW_T_DEFINED\n')
    f.write('typedef struct { int16_t node; int8_t feature; int32_t threshold; int16_t left; int16_t right; int8_t cls; } tree_row_t;\n#endif\n\n')
    f.write(f'#define TREE_NUM_NODES {len(nodes)}\n#define TREE_MAX_DEPTH {clf.get_depth()}\n\n')
    f.write('static const tree_row_t tree_table[] = {\n')
    for n in nodes:
        f.write('    {' + ', '.join(map(str, n)) + '},\n')
    f.write('};\n\n')
    f.write('/* Python prediction for every input in features.h, to check your C code */\n')
    f.write('static const uint8_t python_pred[] = {\n')
    for k in range(0, len(pred), 32):
        f.write('    ' + ' '.join(f'{int(v)},' for v in pred[k:k+32]) + '\n')
    f.write('};\n\n#endif\n')
print('wrote', name)
