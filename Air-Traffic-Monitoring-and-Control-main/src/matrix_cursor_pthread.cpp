#include <pthread.h>
#include <iostream>
#include "cTimer.h"

using namespace std;

struct ThreadArgs {
    int** matrix;      // Pointer to the matrix
    int numRows;       // Number of rows in the matrix
    int numCols;       // Number of columns in the matrix
    int periodSec;     // Desired period of the thread in seconds
    int periodMsec;    // Desired period of the thread in milliseconds
};

void* threadFunction(void* arg) {
    ThreadArgs* args = static_cast<ThreadArgs*>(arg);

    int** matrix = args->matrix;
    int numRows = args->numRows;
    int numCols = args->numCols;
    int periodSec = args->periodSec;
    int periodMsec = args->periodMsec;

    int row = 0, col = 0; // Initialize cursor position

    cTimer timer(periodSec, periodMsec); // Initialize, set, and start the timer

    while (true) {
        matrix[row][col] = 1; // Set the cursor element to 1

        // Print the matrix
        for (int i = 0; i < numRows; ++i) {
            for (int j = 0; j < numCols; ++j) {
                cout << "\t" << matrix[i][j];
            }
            cout << endl;
        }
        cout << endl;

        matrix[row][col] = 0; // Reset the cursor element to 0

        // Move the cursor to the next position
        if (++col == numCols) {
            col = 0;
            if (++row == numRows) {
                break; // Exit if we reach the end of the matrix
            }
        }

        timer.waitTimer();
    }

    return nullptr;
}

