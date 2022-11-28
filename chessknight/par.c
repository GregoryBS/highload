#include <mpi.h>
#include "lib.h"

/*
2 types of process - main and workers.
in the beginning main generates tasks by worker count and put them in task queue.
workers get task from queue and do some job. result send in results queue.
main check result state: if solved - finish, else generate new task(s).
*/

int solve(int **board, int n, int m, int rank, int nprocs)
{
    if (check_sizes(n, m) != OK)
        return NO_SOLVE;

    if (rank == 0)
    {
        int n2 = (n + 1) / 2, m2 = (m + 1) / 2, find = FALSE;
        int i = 0, j = -1;
        while (!find)
        {
            j++;
            if (j == m2)
            {
                j = 0;
                i++;
            }
            if (i == n2)
                break;
            if (n * m % 2 && (i + j) % 2)
                continue;

            /*find = find_solution(board, n, m, i, j, rank, nprocs)) //заменить на параллельную обработку
            if (!find)
                zero_matrix(board, n, m);*/ 
        }
    }
    else 
    {

    }
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
    
    int myrank, nprocs;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);

    for (int i = 0; i < n; i++)
    {
        int **matrix = alloc_matrix(sizes[i][0], sizes[i][1]);
        double time = 0, t = 0;
        for (int j = 0; j < N; j++)
        {
            t = MPI_Wtime();
            solve(matrix, sizes[i][0], sizes[i][1], myrank, nprocs);
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
