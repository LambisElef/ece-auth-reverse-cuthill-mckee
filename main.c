#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define ARRAY_DIM 100

typedef struct {
    int id;
    int degree;
} Node;

void mergeSort(Node* node, int *neighbor, int l, int r);
void merge(Node* node, int *neighbor, int l, int m, int r);

int main() {

    int n = ARRAY_DIM;

    // Creates the symmetric matrix.
    uint8_t *a = (uint8_t *)malloc(n*n*sizeof(uint8_t));

    // Imports symmetric matrix data from external file.
    size_t readStatus;
    char fileName[12];
    sprintf(fileName, "init-%d",n);
    FILE *init = fopen(fileName,"rb");
    readStatus = fread(a, sizeof(uint8_t), n*n, init);
    if (readStatus != n*n)
        printf("Could not read conf-init.bin file.\n");
    fclose(init);

    /*
    // Prints the symmetric array.
    for (int i=0; i<n; i++) {
        for (int j = 0; j < n; j++)
            printf("%d ", a[i*n+j]);
        printf("\n");
    }
    */

    // Creates the nodes.
    Node *node = (Node *)malloc(n*sizeof(Node));

    // Sets the node's ids and calculates their degrees.
    for (int i=0; i<n; i++) {
        node[i].id = i;
        node[i].degree = 0;
        for (int j = 0; j < n; j++)
            node[i].degree += a[i*n+j];
    }

    // Creates the result vector and counter.
    int *res = (int *)malloc(n*sizeof(int));
    int resCounter = 0;

    // Finds the node with the lowest degree.
    Node peripheralNode;
    peripheralNode.id = -1;
    peripheralNode.degree = (int)1e9;
    for (int i=0; i<n; i++)
        if (node[i].degree < peripheralNode.degree)
            peripheralNode = node[i];

    // Adds the peripheral node to the reorder vector.
    res[resCounter++] = peripheralNode.id;

    // Creates the neighbors' ids vector and counter.
    int *neighbor = (int *) malloc(n * sizeof(int));
    int neighborCounter = 0;

    for (int i=0; resCounter<n && i<resCounter; i++) {
        // Initializes the neighbors' ids vector and counter.
        for (int j=0; j<n; j++)
            neighbor[j] = -1;
        neighborCounter = 0;

        // Selects node to look for neighbors.
        int nodeId = res[i];

        // Finds the selected node's neighbors.
        int quit = 0;
        for (int j=0; j<n; j++) {
            quit = 0;
            // Excludes nodes that are already inside of the reorder vector.
            for (int k=0; k<resCounter && !quit; k++)
                // Note that "node[j].id" and "j" represent the same thing, because of the way the node[].id element was created.
                if (j == res[k])
                    quit = 1;
            // Checks if node is a valid neighbor.
            if (a[nodeId*n+j] != 0 && !quit)
                neighbor[neighborCounter++] = j;
        }

        // Sorts the selected node's neighbors by ascending degree.
        mergeSort(node, neighbor, 0, neighborCounter-1);

        // Appends sorted neighbors' ids to result vector.
        for (int j=0; j<neighborCounter; j++)
            res[resCounter++] = neighbor[j];
    }

    /*
    int i;
    for (i=0; i<resCounter; i++)
        printf("%d ", res[i]);
    printf("\n%d ", i);
    */

    // Cleans up.
    free(neighbor);
    free(res);
    free(node);
    free(a);

    return 0;
}

void mergeSort(Node* node, int *neighbor, int l, int r) {
    if (l < r) {

        int m = (l + r) / 2;

        // Sorts first and second halves.
        mergeSort(node, neighbor, l, m);
        mergeSort(node, neighbor, m + 1, r);

        merge(node, neighbor, l, m, r);
    }
}

void merge(Node* node, int *neighbor, int l, int m, int r) {
    int nL = m - l + 1;
    int nR = r - m;

    // Creates temporary arrays L and R.
    int L[nL], R[nR];

    // Copies data to temporary arrays.
    for (int i=0; i<nL; i++)
        L[i] = neighbor[l+i];
    for (int i=0; i<nR; i++)
        R[i] = neighbor[m+1+i];

    // Merges the temporary arrays back into neighbor[l..r].
    int i = 0; // Initial index of first subarray
    int j = 0; // Initial index of second subarray
    int k = l; // Initial index of merged subarray
    while (i<nL && j<nR) {
        if (node[L[i]].degree <= node[R[j]].degree)
            neighbor[k] = L[i++];
        else
            neighbor[k] = R[j++];
        k++;
    }

    // Copies the remaining elements of L, if there are any.
    while (i < nL)
        neighbor[k++] = L[i++];

    // Copies the remaining elements of R, if there are any.
    while (j < nR)
        neighbor[k++] = R[j++];
}