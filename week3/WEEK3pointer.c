/* Week 3: Pointer tree experiments */
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include "features.h"

#ifndef TREE_FILE
#define TREE_FILE "tree_d3.h"
#endif
#include TREE_FILE

#define MAX_NODES 256
#define LEAF -1
#define REPEATS 1000     /* run all inputs this many times */

/* Small node: tight field types, used for the actual tree */
typedef struct node_small {
    int8_t             feature;    /* 0-3 = question, -1 = leaf */
    int8_t             cls;        /* 0 = healthy, 1 = faulty, -1 = none */
    uint16_t           threshold;  /* compare value */
    struct node_small *left;       /* child if feature <= threshold */
    struct node_small *right;      /* child if feature > threshold */
} node_small_t;

/* Wide node: same fields as 32-bit numbers, only for size comparison */
typedef struct node_wide {
    int32_t           feature;
    int32_t           cls;
    int32_t           threshold;
    struct node_wide *left;
    struct node_wide *right;
} node_wide_t;

static node_small_t nodes[MAX_NODES];

/* Build: node n = table row n, then link the pointers */
void build_tree(int num_nodes) {
    for (int i = 0; i < num_nodes; i++) {
        nodes[i].feature   = tree_table[i].feature;
        nodes[i].cls       = tree_table[i].cls;
        nodes[i].threshold = tree_table[i].threshold;

        if (tree_table[i].feature == LEAF) {
            nodes[i].left  = NULL;
            nodes[i].right = NULL;
        } else {
            nodes[i].left  = &nodes[tree_table[i].left];
            nodes[i].right = &nodes[tree_table[i].right];
        }
    }
}

/* Predict, and count how many nodes were visited (including the leaf) */
int pointer_predict(const uint16_t x[4], int *visited) {
    node_small_t *n = &nodes[0];
    int count = 1;
    while (n->feature != LEAF) {
        if (x[n->feature] <= n->threshold) {
            n = n->left;
        } else {
            n = n->right;
        }
        count++;
    }
    *visited = count;
    return n->cls;
}

int main(void) {
    int num_nodes  = (int)(sizeof(tree_table) / sizeof(tree_table[0]));
    build_tree(num_nodes);

    int num_slots  = num_nodes;   /* one node per slot, never empty */
    int empty      = 0;
    int size_small = (int)(sizeof(node_small_t) * num_nodes);
    int size_wide  = (int)(sizeof(node_wide_t)  * num_nodes);

    int mismatches = 0;
    int correct = 0;
    long total_visited = 0;
    int max_visited = 0;

    for (int i = 0; i < NUM_INPUTS; i++) {
        int v;
        int pred = pointer_predict(features[i], &v);
        total_visited += v;
        if (v > max_visited) max_visited = v;
        if (pred != python_pred[i]) mismatches++;
        if (pred == labels[i]) correct++;
    }

    int acc = correct * 1000 / NUM_INPUTS;
    int avg_visited_x100 = (int)(total_visited * 100 / NUM_INPUTS);

    volatile int sink = 0;
    int v;
    clock_t start = clock();
    for (int r = 0; r < REPEATS; r++) {
        for (int i = 0; i < NUM_INPUTS; i++) {
            sink += pointer_predict(features[i], &v);
        }
    }
    clock_t end = clock();

    long long ns_total  = (long long)(end - start) * 1000000000LL / CLOCKS_PER_SEC;
    long long num_preds = (long long)REPEATS * NUM_INPUTS;
    long long ns_x100   = ns_total * 100 / num_preds;

    printf("Tree: %s\n", TREE_FILE);
    printf("Nodes: %d (no empty slots)\n", num_nodes);
    printf("Node size: %d bytes (small), %d bytes (wide)\n",
           (int)sizeof(node_small_t), (int)sizeof(node_wide_t));
    printf("Model size: %d bytes (small fields), %d bytes (32-bit fields)\n", size_small, size_wide);
    printf("Nodes visited per prediction: avg %d.%02d, max %d\n",
           avg_visited_x100 / 100, avg_visited_x100 % 100, max_visited);
    printf("Time per prediction: %lld.%02lld ns\n", ns_x100 / 100, ns_x100 % 100);
    printf("Accuracy: %d.%d%%\n", acc / 10, acc % 10);
    printf("Mismatches vs Python: %d / %d\n\n", mismatches, NUM_INPUTS);

#ifndef NO_CSV
    FILE *f = fopen("week3/results/pointer_results.csv", "a");
    if (f == NULL) {
        printf("Could not open week3/results/pointer_results.csv (does the folder exist?)\n");
        return 1;
    }
    fseek(f, 0, SEEK_END);
    if (ftell(f) == 0) {
        fprintf(f, "tree,nodes,slots,empty_slots,size_small_bytes,size_wide_bytes,"
                   "avg_nodes_visited,max_nodes_visited,ns_per_prediction,accuracy_pct,mismatches\n");
    }
    fprintf(f, "%s,%d,%d,%d,%d,%d,%d.%02d,%d,%lld.%02lld,%d.%d,%d\n",
            TREE_FILE, num_nodes, num_slots, empty, size_small, size_wide,
            avg_visited_x100 / 100, avg_visited_x100 % 100, max_visited,
            ns_x100 / 100, ns_x100 % 100, acc / 10, acc % 10, mismatches);
    fclose(f);
#endif

    return 0;
}