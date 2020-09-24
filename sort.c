/*
 *	File	: sort.c
 *
 *	Author	: Eleftheriadis Charalampos
 *
 *	Date	: 24 September 2020
 */

#include "sort.h"

void quickSort(int *degree, int *neighbor, int l, int r) {
    if (l < r) {
        int pi = partition(degree, neighbor, l, r);

        quickSort(degree, neighbor, l, pi - 1);
        quickSort(degree, neighbor, pi + 1, r);
    }
}

int partition(int *degree, int *neighbor, int l, int r) {
    int pivot = degree[r];
    int strIdx = l;

    for (int i=l; i<r; i++) {
        // If current element is smaller than the pivot.
        if (degree[i] < pivot) {
            swap(&degree[strIdx], &degree[i]);
            swap(&neighbor[strIdx], &neighbor[i]);
            strIdx++;
        }
    }
    swap(&degree[strIdx], &degree[r]);
    swap(&neighbor[strIdx], &neighbor[r]);
    return strIdx;
}

void swap(int* a, int* b) {
    int t = *a;
    *a = *b;
    *b = t;
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