/*
  Gorski, J.; Paquete, L.; Pedrosa, F.
  Greedy algorithms for knapsack problems with binary weights, 2009
  http://eden.dei.uc.pt/~paquete/knap/mpt.c
  Usage: mpt <instance filename>
*/

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <time.h>

int *r, *u, *d; // Arrays to store right, up and diagonal items
int nr, nu, nd; // Numbers of items for each set
int solutions_found = 0; // Global counter of solutions found
FILE *output = NULL;

void put(int row, int col, int val, int rpos, int upos, int dpos) {
	int i;
	if (output != NULL) {
		fprintf(output, "%5d %4d %4d ", val, col, row);
		for (i=0; i<nr; i++) fprintf(output, (i<=rpos) ? "1" : "0");
		for (i=0; i<nu; i++) fprintf(output, (i<=upos) ? "1" : "0");
		for (i=0; i<nd; i++) fprintf(output, (i<=dpos) ? "1" : "0");
		fprintf(output, "\n");
	}
}

int desc_order(const void * a, const void * b) {
  return ( *(int*)b - *(int*)a );
}

int loadFromFile(const char* fileName) {
	FILE* f = fopen(fileName, "r");
	if (f == NULL) {
		printf("Error openning file. Check its existance.");
		return 1;
	}
	
	int N = 0;
	nd = nr = nu = 0;
	char code, buffer[512];
	while(fscanf(f, "%[^\n]\n", buffer) == 1) {
		code = buffer[0];
		if (code == 'n') {
			sscanf(buffer, "n %d", &N);
			r = (int*) malloc(sizeof(int) * N);
			u = (int*) malloc(sizeof(int) * N);
			d = (int*) malloc(sizeof(int) * N);
		} else if (code == 'i') {
			int p, w1, w2;
			sscanf(buffer, "i %d %d %d", &p, &w1, &w2);
			if (w1 == w2 == 1) {
				d[nd] = p;
				nd++;
			} else if (w1 == 1) {
				r[nr] = p;
				nr++;
			} else {
				u[nu] = p;
				nu++;
			}
		}
	}
	
	// We need to sort the elements in non-increasing order
	qsort (r, nr, sizeof(int), desc_order);
	qsort (u, nu, sizeof(int), desc_order);	
	qsort (d, nd, sizeof(int), desc_order);
	
	fclose(f);
	return 0;
}

void freeData() {
	free(r);
	r = NULL;
	free(u);
	u = NULL;
	free(d);
	d = NULL;
}

void buildGrid(int row, int col, int rpos, int upos, int dpos, int profit) {
	
	// We need to know what sector we are working in	
	short int isG3 = (col-row >= nr-nu);
	short int isG2 = (col-row >= 0) && (!isG3);
	short int isG1 = (!isG3) && (!isG2);
	
	int c2 = row, c1 = col, c;
	c = (isG1) ? (c2-c1) : (c1-c2);
	
	while (!(dpos >= nd && (rpos >= nr || upos >= nu))) {
	
		// The cut algorithm (stop here if we know that there will be no non-dominated solutions)
		if ((rpos >= nr || upos >= nu)  // No more up or right elements, only diagonal items
			&& (row >= nu || col >= nr)  // Out of main rectangle
			&& (col-row != nr-nu)) { // Outside of the diagonal
			
			if (isG1 && (r[nu-c] >= d[c2-nu-1])) return;
			
			if (isG2 && (r[c+nu] >= d[c2-nu-1])) return;
			
			if (isG3 && (u[nr-c] >= d[c1-nr-1])) return;
		}
	
		// If there are still super-items, and its a better solution than taking the diagonal, we take that
		// or there isn't diagonal elements left
		if (((rpos < nr && upos < nu) && (r[rpos]+u[upos] >= d[dpos])) || (dpos >= nd)) {
		
			solutions_found++;
			profit += r[rpos] + u[upos];
			put(row, col, profit, rpos, upos, dpos);
			
			upos++;
			rpos++;

		} else { // There are diagonal elements, and its better than taking the super-items
		
			solutions_found++;
			profit += d[dpos];
			put(row, col, profit, rpos, upos, dpos);
		
			dpos++;	
		}
		
		// Move in the diagonal
		row++;
		col++;
		
	}
}

void solve() {
	solutions_found = 0;
	
	// Build the 1-D matrix (basis in each column)
	int temp = 0, i;
	for (i=0; i<=nr; i++) {
		put(0, i, temp, i, 0, 0);
		buildGrid(1, i+1, i, 0, 0, temp);
		temp += r[i];
	}
	
	// Build the 1-D matrix (basis in each row)
	temp = 0;
	for (i=0; i<=nu; i++) {
		put(i, 0, temp, 0, i, 0);
		buildGrid(i+1, 1, 0, i, 0, temp);
		temp += u[i];
	}
}


int main(int argc, char* args[]) {

	if (argc <= 1) { 
		
		printf("No argument given.\n");
		printf(" - You need to pass as argument a instance problem filename\n");
		
	} else {
		
		if (loadFromFile(args[1]) != 0)
			return 1;
		
		// Open a file to save the results
		output = fopen("result.txt", "w");
		
		solve();
		
		fclose(output);
		freeData();
		
	}
	
	return 0;
}
