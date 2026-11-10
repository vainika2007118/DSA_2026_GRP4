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
    int correct = 0;

    for (int i = 0; i < NUM_INPUTS; i++) {
        int pred = baseline_predict(features[i]);

        if (pred != python_pred[i]) {
            mismatches++;
            printf("Mismatch at input %d: C=%d, Python=%d\n", i, pred, python_pred[i]);
            
        }

        if (pred == labels[i]) {
            correct++;
        }
    }

    int acc = (correct * 1000) / NUM_INPUTS;

    printf("Mismatches vs Python: %d / %d\n", mismatches, NUM_INPUTS);
    printf("Accuracy (all inputs): %d.%d%%\n", acc / 10, acc % 10);

    return 0;
}
