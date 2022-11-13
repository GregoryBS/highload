#ifndef LIB_H
#define LIB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRUE 1
#define FALSE 0
#define N 10

#define OK 0
#define NO_SOLVE -1

void free_matrix(int **matrix, int n);

int **alloc_matrix(int n, int m);

void zero_matrix(int **matrix, int n, int m);

void print_matrix(FILE *f, int **matrix, int n, int m);

int steps[][2] = {{-1, -2}, {-2, -1}, {-2, 1}, {1, -2}, {-1, 2}, {2, -1}, {1, 2}, {2, 1}};

#endif
