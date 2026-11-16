#include <stdio.h>
#include <stdint.h>
#include "features.h"
#define TREE_FILE "tree_d3.h" /*similarly d5,d8 and d12*/
#include TREE_FILE

#define MAX_SLOTS 8191   /* depth 12: 2^(12+1) - 1 */
#define LEAF  -1         /* feature value that means "leaf" */
#define EMPTY -2         /* feature value that means "empty box" */

/* One box of the array tree: no left/right, position does that job */
typedef struct {
    int16_t  feature;    /* 0-3 = question, -1 = leaf, -2 = empty */
    uint16_t threshold;  /* compare value for questions */
    int16_t  cls;        /* answer for leaves: 0 healthy, 1 faulty */
} slot_t;

static slot_t slots[MAX_SLOTS];

void clear_slots(void) {
    for (int i = 0; i < MAX_SLOTS; i++) {
        slots[i].feature = EMPTY;
        slots[i].threshold = 0;
        slots[i].cls = -1;
    }
}

void place(int row, int box) {
    slots[box].feature   = tree_table[row].feature;
    slots[box].threshold = tree_table[row].threshold;
    slots[box].cls       = tree_table[row].cls;

    if (tree_table[row].feature != LEAF) {
        place(tree_table[row].left,  2 * box + 1);
        place(tree_table[row].right, 2 * box + 2);
    }
}

/* Predict by moving to 2i+1 (yes) or 2i+2 (no) until a leaf */
int array_predict(const uint16_t x[4]) {
    int i = 0;
    while (slots[i].feature != LEAF) {
        if (x[slots[i].feature] <= slots[i].threshold) {
            i = 2 * i + 1;
        } else {
            i = 2 * i + 2;
        }
    }
    return slots[i].cls;
}

int main(void) {
    printf("Running tree: %s\n", TREE_FILE);
    clear_slots();
    place(0, 0);                    

    int mismatches = 0;
    for (int i = 0; i < NUM_INPUTS; i++) {
        int pred = array_predict(features[i]);
        if (pred != python_pred[i]) {
            mismatches++;
            if (mismatches <= 5) {
                printf("Mismatch at input %d: C=%d, Python=%d\n", i, pred, python_pred[i]);
            }
        }
    }

    printf("Mismatches vs Python: %d / %d\n", mismatches, NUM_INPUTS);
    return 0;
}