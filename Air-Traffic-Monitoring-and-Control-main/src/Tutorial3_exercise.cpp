#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
using namespace std;
#include "cTimer.h"

// Structure to pass arguments to the thread function
struct thread_args {
	int** location;      // Pointer to the matrix (2D array)
	int n_row;           // Number of rows in the matrix
	int n_col;           // Number of columns in the matrix
	int period_sec;      // Desired period of the thread in seconds
	int period_msec;     // Desired period of the thread in milliseconds
};

// Thread function to control a cursor moving through the matrix
void *thread_start (void *arg) {
	struct thread_args *targs = (struct thread_args *) arg; // Cast argument to thread_args pointer

	int **loc = targs->location; // Matrix location
	int n_row = targs->n_row;    // Number of rows in the matrix
	int n_col = targs->n_col;    // Number of columns in the matrix

	int period_sec = targs->period_sec;    // Timer period in seconds
	int period_msec = targs->period_msec;  // Timer period in milliseconds

	int i_pos = 0, j_pos = 0; // Initializing cursor indices

	// Initialize, set, and start the timer with the specified period
	cTimer timer(period_sec, period_msec);

	while (true) {
		// Set the current cursor position to 1
		*(*(loc + i_pos) + j_pos) = 1;

		// Print the current state of the matrix
		int **p; // Pointer to iterate through matrix rows
		for (int i = 0; i < n_row; i++) {
			p = loc + i; // Set pointer to the start of each row
			for (int j = 0; j < n_col; j++)
				cout << "\t" << *(*p + j); // Print each element in the row
			cout << endl;
		}
		cout << endl;

		// Reset the current cursor position to 0
		*(*(loc + i_pos) + j_pos) = 0;

		// Move the cursor to the next column
		j_pos++;
		// If end of row is reached, move to the next row
		if (j_pos == n_col) {
			j_pos = 0; // Reset column index
			i_pos++;   // Move to next row
		}
		// If end of matrix is reached, exit the loop
		if (i_pos == n_row) {
			break;
		}

		timer.waitTimer(); // Wait for the timer period before next iteration
	}

	return NULL; // Exit the thread
}
