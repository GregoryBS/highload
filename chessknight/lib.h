#ifndef LIB_H
#define LIB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRUE 1
#define FALSE 0
#define N 1
#define STEPS 8
#define FSTEPS 2

#define OK 0
#define NO_SOLVE -1

void free_matrix(int **matrix, int n);

int **alloc_matrix(int n, int m);

void zero_matrix(int **matrix, int n, int m);

void print_matrix(FILE *f, int **matrix, int n, int m);

int check_sizes(int n, int m);

int make_step(int *step_done, int step, int n, int m, int i, int j, int pos_i, int pos_j);

int find_path(int *step_done, int n, int m, int i, int j, int step, int max_step);

void fill_board(int **board, int i, int j, int *step_done, int step);

#endif
