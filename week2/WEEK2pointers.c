/* Week 2: Pointer tree
 * Each node is stored in a fixed array of structs and linked to its
 * children by left/right pointers. Checks all 2000 inputs against Python.
 */
#include <stdio.h>
#include <stdint.h>
#include "features.h"

#define TREE_FILE "tree_d3.h"
#include TREE_FILE

#define MAX_NODES 256    
#define LEAF -1

/* One node: question/answer plus pointers to its children */
typedef struct node {
    int16_t      feature;    /* 0-3 = question, -1 = leaf */
    uint16_t     threshold;  
    int16_t      cls;        
    struct node *left;       
    struct node *right;      
} node_t;

static node_t nodes[MAX_NODES];

/* Build: node n = table row n, then link the pointers */
void build_tree(int num_nodes) {
    for (int i = 0; i < num_nodes; i++) {
        nodes[i].feature   = tree_table[i].feature;
        nodes[i].threshold = tree_table[i].threshold;
        nodes[i].cls       = tree_table[i].cls;

        if (tree_table[i].feature == LEAF) {
            nodes[i].left  = NULL;
            nodes[i].right = NULL;
        } else {
            nodes[i].left  = &nodes[tree_table[i].left];
            nodes[i].right = &nodes[tree_table[i].right];
        }
    }
}

/* Predict: start at the root and follow pointers until a leaf */
int pointer_predict(const uint16_t x[4]) {
    node_t *n = &nodes[0];
    while (n->feature != LEAF) {
        if (x[n->feature] <= n->threshold) {
            n = n->left;
        } else {
            n = n->right;
        }
    }
    return n->cls;
}

int main(void) {
    int num_nodes = (int)(sizeof(tree_table) / sizeof(tree_table[0]));
    build_tree(num_nodes);

    int mismatches = 0;
    for (int i = 0; i < NUM_INPUTS; i++) {
        int pred = pointer_predict(features[i]);
        if (pred != python_pred[i]) {
            mismatches++;
            if (mismatches <= 5) {
                printf("Mismatch at input %d: C=%d, Python=%d\n", i, pred, python_pred[i]);
            }
        }
    }

    printf("Running tree: %s\n", TREE_FILE);
    printf("Mismatches vs Python: %d / %d\n", mismatches, NUM_INPUTS);
    return 0;
}