#include <mpi.h>
#include "lib.h"

int solve(int **board, int n, int m)
{
    return OK;
}

int main(int argc, char **argv)
{
    int sizes[][2] = {{3, 4}, {4, 5}, {5, 5}, {3, 7}, {7, 7}, {8, 8}, /*{10, 10}, {3, 14}, {15, 15}, {25, 25}, 
                    {39, 39}, {50, 50}, {64, 64}, {73, 73}, {100, 100}, {200, 200}, {500, 500}, {1000, 1000}*/};
    int n = sizeof(sizes) / sizeof(sizes[0]);
    FILE *file = stdout;
    if (!file)
        return NO_SOLVE;
    
    MPI_Init(&argc, &argv);
    for (int i = 0; i < n; i++)
    {
        int **matrix = alloc_matrix(sizes[i][0], sizes[i][1]);
        double time = 0, t = 0;
        for (int j = 0; j < N; j++)
        {
            t = MPI_Wtime();
            solve(matrix, sizes[i][0], sizes[i][1]);
            time += MPI_Wtime() - t;
            if (j < N - 1)
                zero_matrix(matrix, sizes[i][0], sizes[i][1]);
        }
        fprintf(file, "Solution for %dx%d board:\n", sizes[i][0], sizes[i][1]);
        print_matrix(file, matrix, sizes[i][0], sizes[i][1]);
        fprintf(file, "Time for solution: %lf\n", time / N);
        free_matrix(matrix, sizes[i][0]);
    }
    MPI_Finalize();
    fclose(file);
    return OK;
}
