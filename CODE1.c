#include <stdio.h>
#include <stdint.h>
#include "features.h"
#include "tree_d3.h"

/*
 * Features: x[0]=RMS, x[1]=PEAK, x[2]=CREST_X100, x[3]=KURT_X100
 * Returns: 0 = healthy, 1 = faulty
 */
int baseline_predict(const uint16_t x[4]) 
{
    if (x[1] <= 512) {
        if (x[3] <= 519) {
            if (x[2] <= 429) {
                return 0;
            } else {
                return 1;
            }
        } else {
            if (x[3] <= 588) {
                return 1;
            } else {
                return 1;
            }
        }
    } else {
        if (x[3] <= 364) {
            if (x[2] <= 373) {
                return 0;
            } else {
                return 1;
            }
        } else {
            if (x[3] <= 492) {
                return 1;
            } else {
                return 1;
            }
        }
    }
}

int main(void) 
{
    int mismatches = 0;
    int correct_all = 0;
    int correct_test = 0;
    int test_total = NUM_INPUTS - NUM_TRAIN;

    for (int i = 0; i < NUM_INPUTS; i++) {
        int pred = baseline_predict(features[i]);

        if (pred != python_pred[i]) {
            mismatches++;
            if (mismatches <= 5) {
                printf("Mismatch at input %d: C=%d, Python=%d\n", i, pred, python_pred[i]);
            }
        }

        if (pred == labels[i]) {
            correct_all++;
            if (i >= NUM_TRAIN) {
                correct_test++;
            }
        }
    }

    int acc_all = (correct_all * 1000) / NUM_INPUTS;
    int acc_test = (correct_test * 1000) / test_total;

    printf("Mismatches vs Python: %d / %d\n", mismatches, NUM_INPUTS);
    printf("Accuracy (all inputs): %d.%d%%\n", acc_all / 10, acc_all % 10);
    printf("Accuracy (test inputs): %d.%d%%\n", acc_test / 10, acc_test % 10);

    return 0;
}
