/* Week 3: Array tree experiments */
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include "features.h"

#ifndef TREE_FILE
#define TREE_FILE "tree_d3.h"   
#endif
#include TREE_FILE

#define MAX_SLOTS 8191  
#define LEAF  -1
#define EMPTY -2
#define REPEATS 1000     /* run all inputs this many times*/


typedef struct {
    int8_t   feature;    /* 0-3 = question, -1 = leaf, -2 = empty */
    int8_t   cls;        /* 0 = healthy, 1 = faulty, -1 = none */
    uint16_t threshold;  /* compare value */
} slot_small_t;


typedef struct {
    int32_t feature;
    int32_t cls;
    int32_t threshold;
} slot_wide_t;

static slot_small_t slots[MAX_SLOTS];
static int last_box = 0;   /* highest box index used */

/* Mark every box empty */
void clear_slots(void) {
    for (int i = 0; i < MAX_SLOTS; i++) {
        slots[i].feature = EMPTY;
        slots[i].cls = -1;
        slots[i].threshold = 0;
    }
}

/* Copy table row into its box, then seat its children at 2i+1 and 2i+2 */
void place(int row, int box) {
    slots[box].feature   = tree_table[row].feature;
    slots[box].cls       = tree_table[row].cls;
    slots[box].threshold = tree_table[row].threshold;

    if (box > last_box) last_box = box;

    if (tree_table[row].feature != LEAF) {
        place(tree_table[row].left,  2 * box + 1);
        place(tree_table[row].right, 2 * box + 2);
    }
}

/* Predict, and count how many boxes were visited (including the leaf) */
int array_predict(const uint16_t x[4], int *visited) {
    int i = 0;
    int count = 1;
    while (slots[i].feature != LEAF) {
        if (x[slots[i].feature] <= slots[i].threshold) {
            i = 2 * i + 1;
        } else {
            i = 2 * i + 2;
        }
        count++;
    }
    *visited = count;
    return slots[i].cls;
}

int main(void) {
    clear_slots();
    place(0, 0);

    int num_nodes  = (int)(sizeof(tree_table) / sizeof(tree_table[0]));
    int num_slots  = last_box + 1;
    int empty      = num_slots - num_nodes;
    int size_small = (int)(sizeof(slot_small_t) * num_slots);
    int size_wide  = (int)(sizeof(slot_wide_t)  * num_slots);

    
    int mismatches = 0;
    int correct = 0;
    long total_visited = 0;
    int max_visited = 0;

    for (int i = 0; i < NUM_INPUTS; i++) {
        int v;
        int pred = array_predict(features[i], &v);
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
            sink += array_predict(features[i], &v);
        }
    }
    clock_t end = clock();

    long long ns_total  = (long long)(end - start) * 1000000000LL / CLOCKS_PER_SEC;
    long long num_preds = (long long)REPEATS * NUM_INPUTS;
    long long ns_x100   = ns_total * 100 / num_preds;          

   
    printf("Tree: %s\n", TREE_FILE);
    printf("Nodes: %d, slots: %d, empty slots: %d\n", num_nodes, num_slots, empty);
    printf("Model size: %d bytes (small fields), %d bytes (32-bit fields)\n", size_small, size_wide);
    printf("Nodes visited per prediction: avg %d.%02d, max %d\n",
           avg_visited_x100 / 100, avg_visited_x100 % 100, max_visited);
    printf("Time per prediction: %lld.%02lld ns\n", ns_x100 / 100, ns_x100 % 100);
    printf("Accuracy: %d.%d%%\n", acc / 10, acc % 10);
    printf("Mismatches vs Python: %d / %d\n\n", mismatches, NUM_INPUTS);

    
    FILE *f = fopen("results/array_results.csv", "a");
    if (f == NULL) {
        printf("Could not open results/array_results.csv (does the results folder exist?)\n");
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

    return 0;
}