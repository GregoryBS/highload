#include "lib.h"

void free_matrix(int **matrix, int n)
{
    for (int i = 0; i < n; i++)
        free(matrix[i]);
    free(matrix);
}

int **alloc_matrix(int n, int m)
{
    int **ptr = calloc(n, sizeof(int *));
    if (!ptr)
        return NULL;

    for (int i = 0; i < n; i++)
    {
        ptr[i] = calloc(m, sizeof(int));
        if (!ptr[i])
        {
            free_matrix(ptr, n);
            return NULL;
        }
    }
    return ptr;
}

void zero_matrix(int **matrix, int n, int m)
{
    for (int i = 0; i < n; i++)
        memset(matrix[i], 0, m * sizeof(int));
}

void print_matrix(FILE *f, int **matrix, int n, int m)
{
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
            fprintf(f, "%8d", matrix[i][j]);
        fprintf(f, "\n");
    }
}
