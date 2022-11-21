#include <mpi.h>
#include "lib.h"

int steps[][2] = {{-1, -2}, {-2, -1}, {-2, 1}, {1, -2}, {-1, 2}, {2, -1}, {1, 2}, {2, 1}};

/*
2 types of process - main and workers.
2 queues - tasks and results.
in the beginning main generates tasks by worker count and put them in task queue.
workers get task from queue and do some job. result send in results queue.
main check result state: if solved - finish, else generate new task(s).
*/

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
    /*for (int i = 0; i < n; i++)
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
    }*/
    int myrank, nprocs, len;
	char name[MPI_MAX_PROCESSOR_NAME];

    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
	MPI_Comm_rank(MPI_COMM_WORLD, &myrank);
	MPI_Get_processor_name(name, &len);
    printf("Hello from processor %s[%d] %d of %d  \n", name, len, myrank, nprocs);
    MPI_Finalize();
    fclose(file);
    return OK;
}
