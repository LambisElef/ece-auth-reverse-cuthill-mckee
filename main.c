/*
 *	File	: main.c
 *
 *	Author	: Eleftheriadis Charalampos
 *
 *	Date	: 31 July 2020
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include "sort.h"

#define OPENMP 0
#define BENCH_SECTORS 1
#define BENCH_SORT 0

int main() {

    // Initializes variables.
    int mode = 0;
    char sparseArrayName[32];
    char fileName[64];
    int n = 0;
    int totalElements = 0;

    // Selects mode (auto or manual)
    printf("Select mode: \n 0 - Manual\n 1 - filter3D\n 2 - F1\n 3 - audikw_1\n");
    scanf("%d", &mode);

    if (mode == 0) {
        // Selects sparse matrix file.
        printf("Enter sparse array name: ");
        scanf("%s", sparseArrayName);

        // Selects dimension of matrix.
        printf("Enter matrix dimension: ");
        scanf("%d", &n);
        if (n <= 0) {
            printf("Bad dimension size!\n");
            return -1;
        }

        // Selects number of non-zero elements.
        printf("Enter number of non-zero elements: ");
        scanf("%d", &totalElements);
        if (totalElements <= 0) {
            printf("Bad dimension size!\n");
            return -1;
        }
    } else if (mode == 1) {
        sprintf(sparseArrayName, "filter3D");
        n = 106437;
        totalElements = 2707179;
    } else if (mode == 2) {
        sprintf(sparseArrayName, "F1");
        n = 343791;
        totalElements = 26837113;
    } else if (mode == 3) {
        sprintf(sparseArrayName, "audikw_1");
        n = 943695;
        totalElements = 77651847;
    } else {
        printf("Bad mode selection!\n");
        return -1;
    }

    // Creates two arrays containing the non-zero elements' indexes.
    int *x = (int *)malloc(totalElements*sizeof(int));
    int *y = (int *)malloc(totalElements*sizeof(int));

    // Imports the sparse matrix non-zero elements' indexes from external file.
    sprintf(fileName, "%s.csv", sparseArrayName);
    FILE *init = fopen(fileName,"r");
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
    struct timeval start, end;
    gettimeofday(&start, NULL);

    #if BENCH_SECTORS
    struct timeval startSector[6], endSector[6];
    gettimeofday(&startSector[0], NULL);
    #endif

    #if BENCH_SORT
    struct timeval startSort, endSort;
    int timeSort = 0;
    #endif

    // Creates the nodes' degrees vector.
    int *degree = (int *)malloc(n*sizeof(int));

    // Creates the nodes' flags vector that indicate if a node is added to the result vector.
    int8_t *inRes = (int8_t *)malloc(n*sizeof(int8_t));

    // Creates the nodes' edges counters vector that indicate how many of a node's edges have been added to the transformed sparse matrix.
    int *edgeCounter = (int *)malloc(n*sizeof(int));

    #if BENCH_SECTORS
    gettimeofday(&endSector[0], NULL);
    gettimeofday(&startSector[1], NULL);
    #endif

    // Initializes the nodes' degrees, flags and neighbor counters vectors.
    for (int i=0; i<n; i++) {
        degree[i] = 0;
        inRes[i] = 0;
        edgeCounter[i] = 0;
    }

    #if BENCH_SECTORS
    gettimeofday(&endSector[1], NULL);
    gettimeofday(&startSector[2], NULL);
    #endif

    // Calculates the nodes' degrees that are equal to the nodes' edges.
    #if OPENMP
    #pragma omp parallel for reduction(+: degree[:n])
    #endif
    for (int i=0; i<totalElements; i++)
        degree[y[i]]++;

    #if BENCH_SECTORS
    gettimeofday(&endSector[2], NULL);
    gettimeofday(&startSector[3], NULL);
    #endif

    // Transforms the sparse matrix.
    int **aT = (int **)malloc(n*sizeof(int *));
    aT[0] = (int *)malloc(totalElements*sizeof(int));
    for (int i=1; i<n; i++)
        aT[i] = aT[i-1] + degree[i-1];
    int *counterDeg = (int *)malloc(n*sizeof(int));
    counterDeg[0] = 0;
    for (int i=1; i<n; i++)
        counterDeg[i] = counterDeg[i-1] + degree[i-1];
    #if OPENMP
    #pragma omp parallel for
    #endif
    for (int i=0; i<n; i++)
        for (int j=0; j<degree[i]; j++)
            aT[y[counterDeg[i]]][edgeCounter[y[counterDeg[i]]]++] = x[counterDeg[i]+j];

    #if BENCH_SECTORS
    gettimeofday(&endSector[3], NULL);
    gettimeofday(&startSector[4], NULL);
    #endif

    // Creates the result vector and counter.
    int *res = (int *)malloc(n*sizeof(int));
    int resCounter = 0;

    // Creates the neighbors' ids vector and counter.
    int *neighbor = (int *)malloc(n*sizeof(int));
    int neighborCounter = 0;

    #if BENCH_SECTORS
    gettimeofday(&endSector[4], NULL);
    gettimeofday(&startSector[5], NULL);
    #endif

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

            #if BENCH_SORT
            gettimeofday(&startSort, NULL);
            #endif

            // Sorts the selected node's neighbors by ascending degree.
            mergeSort(degree, neighbor, 0, neighborCounter-1);

            #if BENCH_SORT
            gettimeofday(&endSort, NULL);
            timeSort += (endSort.tv_sec-startSort.tv_sec)*(int)1e6 + endSort.tv_usec-startSort.tv_usec;
            #endif

            // Appends sorted neighbors' ids to result vector.
            for (int j=0; j<neighborCounter; j++) {
                res[resCounter++] = neighbor[j];
                inRes[neighbor[j]] = 1;
            }
        }
    }

    #if BENCH_SECTORS
    gettimeofday(&endSector[5], NULL);
    #endif

    // Saves a timestamp when the algorithm ends and calculates the elapsed time in useconds.
    gettimeofday(&end, NULL);
    long elapsedTime = (end.tv_sec-start.tv_sec)*(long)1e6 + end.tv_usec-start.tv_usec;

    // Calculates elapsed sectors' times.
    #if BENCH_SECTORS
    long elapsedTimeSectors[6];
    for (int i=0; i<6; i++)
        elapsedTimeSectors[i] = (endSector[i].tv_sec-startSector[i].tv_sec)*(long)1e6 + endSector[i].tv_usec-startSector[i].tv_usec;
    #endif

    // Writes res vector to external file.
    #if OPENMP
    sprintf(fileName, "%s-OMPres.csv", sparseArrayName);
    #else
    sprintf(fileName, "%s-res.csv", sparseArrayName);
    #endif
    FILE *fileRes = fopen(fileName,"w");
    for (int i=0; i<n; i++)
        fprintf(fileRes, "%d,", res[n-1-i]);
    fprintf(fileRes, "\n");
    fclose(fileRes);

    // Writes execution time to file.
    #if OPENMP
    sprintf(fileName, "%s-OMPtime.csv", sparseArrayName);
    #else
    sprintf(fileName, "%s-time.csv", sparseArrayName);
    #endif
    FILE *fileTime = fopen(fileName,"a");
    fprintf(fileTime, "%ld", elapsedTime);
    fprintf(fileTime, "\n");
    fclose(fileTime);

    // Writes execution sectors' times to file.
    #if BENCH_SECTORS
    #if OPENMP
    sprintf(fileName, "%s-OMPtimeSectors.csv", sparseArrayName);
    #else
    sprintf(fileName, "%s-timeSectors.csv", sparseArrayName);
    #endif
    FILE *fileTimeSectors = fopen(fileName,"a");
    for (int i=0; i<6; i++) {
        fprintf(fileTimeSectors, "%ld", elapsedTimeSectors[i]);
        if (i != 5)
            fprintf(fileTimeSectors, ",");
    }
    fprintf(fileTimeSectors, "\n");
    fclose(fileTimeSectors);
    #endif

    // Cleans up. Some variables can be freed earlier, but that would hurt the measured performance when comparing with Matlab.
    free(neighbor);
    free(res);
    free(aT[0]);
    free(aT);
    free(edgeCounter);
    free(inRes);
    free(counterDeg);
    free(degree);
    free(y);
    free(x);

    return 0;
}
