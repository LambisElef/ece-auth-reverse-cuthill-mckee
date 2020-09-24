/*
 *	File	: sort.h
 *
 *	Author	: Eleftheriadis Charalampos
 *
 *	Date	: 24 September 2020
 */

#ifndef REVERSE_CUTHILL_MCKEE_SORT_H
#define REVERSE_CUTHILL_MCKEE_SORT_H

void swap(int* a, int* b);
void quickSort(int *degree, int *neighbor, int l, int r);
int partition(int *degree, int *neighbor, int l, int r);
void mergeSort(int* degree, int *neighbor, int l, int r);
void merge(int* degree, int *neighbor, int l, int m, int r);

#endif //REVERSE_CUTHILL_MCKEE_SORT_H
