/*
 *	File	: main_sequential.c
 *
 *	Author	: Eleftheriadis Charalampos
 *
 *	Date	: 31 July 2020
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

void mergeSort(int* degree, int *neighbor, int l, int r);
void merge(int* degree, int *neighbor, int l, int m, int r);

int main() {

    // Selects sparse matrix file.
    char initFileName[32];
    printf("Enter sparse matrix file name: ");
    scanf("%s", initFileName);

    // Selects dimension of matrix.
    int n = 0;
    printf("Enter matrix dimension: ");
    scanf("%d", &n);
    if (n <= 0) {
        printf("Bad dimension size!\n");
        return -1;
    }

    // Selects number of non-zero elements.
    int totalElements = 0;
    printf("Enter number of non-zero elements: ");
    scanf("%d", &totalElements);
    if (totalElements <= 0) {
        printf("Bad dimension size!\n");
        return -1;
    }

    // Creates two arrays containing the non-zero elements' indexes.
    int *x = (int *)malloc(totalElements*sizeof(int));
    int *y = (int *)malloc(totalElements*sizeof(int));

    // Imports the sparse matrix non-zero elements' indexes from external file.
    FILE *init = fopen(initFileName,"r");
    if(init == NULL) {
        fprintf(stderr, "Cannot open file.\n");
        return -1;
    }
    char line[128];
    int xTemp, yTemp, xyCounter = 0;
    size_t lineNo = 0;
    while(fgets(line, sizeof(line), init)) {
        lineNo++;
        if(sscanf(line, "%d,%d", &xTemp, &yTemp) != 2) {
            fprintf(stderr, "Format error on line %zu.\n", lineNo);
            return -1;
        }
        // Subtracts 1 from index so that elements' indexes begin from 0 instead of 1.
        x[xyCounter] = xTemp - 1;
        y[xyCounter] = yTemp - 1;
        xyCounter++;
    }
    fclose(init);

    // Saves a timestamp when the algorithm begins.
    struct timeval start[3], end[3];
    gettimeofday(&start[0], NULL);

    // Creates the nodes' degrees vector.
    int *degree = (int *)malloc(n*sizeof(int));

    // Creates the nodes' flags vector that indicate if a node is added to the result vector.
    int8_t *inRes = (int8_t *)malloc(n*sizeof(int8_t));

    // Creates the nodes' edges counters vector that indicate how many of a node's edges have been added to the transformed sparse matrix.
    int *edgeCounter = (int *)malloc(n*sizeof(int));

    // Initializes the nodes' degrees, flags and neighbor counters vectors.
    for (int i=0; i<n; i++) {
        degree[i] = 0;
        inRes[i] = 0;
        edgeCounter[i] = 0;
    }

    // Calculates the nodes' degrees that are equal to the nodes' edges.
    for (int i=0; i<totalElements; i++)
        degree[x[i]]++1;

    gettimeofday(&end[0], NULL);
    gettimeofday(&start[1], NULL);

    // Transforms the sparse matrix.
    int **aT = (int **)malloc(n*sizeof(int *));
    aT[0] = (int *)malloc(totalElements*sizeof(int));
    for (int i=1; i<n; i++)
        aT[i] = aT[i-1] + degree[i-1];
    for (int i=0; i<totalElements; i++)
        aT[x[i]][edgeCounter[x[i]]++] = y[i];

    gettimeofday(&end[1], NULL);
    gettimeofday(&start[2], NULL);

    // Creates the result vector and counter.
    int *res = (int *)malloc(n*sizeof(int));
    int resCounter = 0;

    // Creates the neighbors' ids vector and counter.
    int *neighbor = (int *)malloc(n*sizeof(int));
    int neighborCounter = 0;

    // This while loop is needed in case there are disjoint graphs.
    while (resCounter < n) {
        // Finds the node with the lowest degree.
        int peripheralNodeId = -1;
        int peripheralNodeDegree = (int)1e9;
        for (int i=0; i<n; i++)
            if (inRes[i] != 1 && degree[i] < peripheralNodeDegree) {
                peripheralNodeId = i;
                peripheralNodeDegree = degree[i];
            }

        // Adds the peripheral node to the reorder vector.
        res[resCounter++] = peripheralNodeId;
        inRes[peripheralNodeId] = 1;

        // This for loop iterates through the nodes contained in the result vector.
        for (int i=0; resCounter<n && i<resCounter; i++) {
            // Initializes the neighbors' ids counter.
            neighborCounter = 0;

            // Selects node to look for neighbors.
            int nodeId = res[i];

            // Finds the selected node's neighbors.
            for (int j=0; j<degree[nodeId]; j++)
                if (inRes[aT[nodeId][j]] != 1)
                    neighbor[neighborCounter++] = aT[nodeId][j];

            // Sorts the selected node's neighbors by ascending degree.
            mergeSort(degree, neighbor, 0, neighborCounter-1);

            // Appends sorted neighbors' ids to result vector.
            for (int j=0; j<neighborCounter; j++) {
                res[resCounter++] = neighbor[j];
                inRes[neighbor[j]] = 1;
            }
        }
    }

    // Saves a timestamp when the algorithm ends and calculates the elapsed time in useconds.
    gettimeofday(&end[2], NULL);
    int elapsedTime[3];
    for (int i=0; i<3; i++)
        elapsedTime[i] = (end[i].tv_sec-start[i].tv_sec)*(int)1e6 + end[i].tv_usec-start[i].tv_usec;

    // Writes res vector to external file.
    char fileName[64];
    sprintf(fileName, "res-%s", initFileName);
    FILE *finalRes = fopen(fileName,"w");
    for (int i=0; i<n; i++)
        fprintf(finalRes, "%d,", res[n-1-i]);
    fclose(finalRes);

    // Writes execution time to file.
    sprintf(fileName, "time-%s",initFileName);
    FILE *time = fopen(fileName,"w");
    for (int i=0; i<3; i++)
        fprintf(time, "%d,", elapsedTime[i]);
    fclose(time);

    // Cleans up. Some variables can be freed earlier, but that would hurt the measured performance when comparing with Matlab.
    free(neighbor);
    free(res);
    free(aT[0]);
    free(aT);
    free(edgeCounter);
    free(inRes);
    free(degree);
    free(y);
    free(x);

    return 0;
}

void mergeSort(int* degree, int *neighbor, int l, int r) {
    if (l < r) {

        int m = (l + r) / 2;

        // Sorts first and second halves.
        mergeSort(degree, neighbor, l, m);
        mergeSort(degree, neighbor, m + 1, r);

        merge(degree, neighbor, l, m, r);
    }
}

void merge(int* degree, int *neighbor, int l, int m, int r) {
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
        if (degree[L[i]] <= degree[R[j]])
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
