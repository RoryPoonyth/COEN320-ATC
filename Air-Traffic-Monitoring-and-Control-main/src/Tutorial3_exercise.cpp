#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
using namespace std;
#include "cTimer.h"

struct thread_args {
    int** location;      /* Location of the row */
    int n_row;           /* Number of rows of the matrix */
    int n_col;           /* Number of columns of the matrix */
    int period_sec;      // Desired period of the thread in seconds
    int period_msec;     // Desired period of the thread in milliseconds
};

void* thread_start(void* arg) {
    struct thread_args* targs = (struct thread_args*)arg;

    int** loc = targs->location;
    int n_row = targs->n_row;
    int n_col = targs->n_col;
    int period_sec = targs->period_sec;
    int period_msec = targs->period_msec;

    int i_pos = 0, j_pos = 0; // Initializing the indexes of the cursor

    cTimer timer(period_sec, period_msec); // Initialize, set, and start the timer

    while (true) {
        *(*(loc + i_pos) + j_pos) = 1; // Setting the cursor element to 1

        // Print the matrix
        for (int i = 0; i < n_row; i++) {
            for (int j = 0; j < n_col; j++) {
                cout << "\t" << loc[i][j];
            }
            cout << endl;
        }
        cout << endl;

        *(*(loc + i_pos) + j_pos) = 0; // Resetting the cursor element to 0

        // Incrementing the index of the cursor
        j_pos++;
        if (j_pos == n_col) { // If j_pos == n_col, the cursor moves to the next line
            j_pos = 0;
            i_pos++;
        }
        if (i_pos == n_row) { // If i_pos == n_row, the cursor has reached the end of the matrix
            break;
        }

        timer.waitTimer();
    } // End of while loop

    return NULL;
} // End of thread_start()
