#include <mpi.h>
#include "lib.h"

#ifdef _OPENMP

#include <omp.h>

#endif

extern int steps[STEPS][2];

int solve(int **board, int n, int m, int rank, int nprocs)
{
    if (check_sizes(n, m) != OK)
        return NO_SOLVE;
    int max_step = n * m - 1;
    int *step_done = malloc(max_step * sizeof(int));
    if (!step_done)
        return NO_SOLVE; 

    if (rank == 0) 
    {
        int n2 = (n + 1) / 2, m2 = (m + 1) / 2, find = FALSE;
        int i = 0, j = -1, stepsk = sizeof(steps) / sizeof(steps[0]);
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

            int **tasks = alloc_matrix(STEPS * STEPS, 4);
            int first = -1, second = -1, count = 0;
            while (1)
            {
                if (second < 0)
                {
                    first = make_step(board, n, m, i, j, first);
                    if (first < 0)
                    {
                        board[i][j] = 0;
                        break;
                    }
                    board[i][j] = 1;
                }
                second = make_step(board, n, m, i + steps[first][0], j + steps[first][1], second);
                if (second < 0)
                    continue;
                tasks[count][0] = i; tasks[count][1] = j; tasks[count][2] = first; tasks[count][3] = second;
                count++;
            }
            int k = 0, x = 1;
#pragma omp parallel default(none) shared(board, i, j, find, tasks, count, steps, max_step, nprocs) private(k, x, step_done)
            {
#pragma omp for
                for (k = 0; k < count; k++)
                {
                    if (!find)
                    {
                        MPI_Send(tasks[k], 4, MPI_INT, k % nprocs + 1, 0, MPI_COMM_WORLD);
                        MPI_Status status;
                        MPI_Recv(step_done, max_step, MPI_INT, k % nprocs + 1, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
#pragma omp critical
                        {
                            if (status.MPI_TAG)
                            {
                                find = status.MPI_TAG;
                                board[i][j] = 1;
                                for (int k0 = 0; k0 < max_step; k0++)
                                {
                                    i += steps[step_done[k]][0];
                                    j += steps[step_done[k]][1];
                                    board[i][j] = k0 + 2;
                                }
                            }
                        }
                    }
                }
            }
            free_matrix(tasks, STEPS * STEPS);
            for (k = 1; k < nprocs; k++)
                MPI_Send(&k, 1, MPI_INT, k, 1, MPI_COMM_WORLD);
        }
    }
    else 
    {
        while (1)
        {
            int task[4];
            MPI_Status status;
            MPI_Recv(task, 4, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            if (status.MPI_TAG)
                break;
            int i = task[0], j = task[1], f = task[2], s = task[3];
            step_done[0] = f;
            step_done[1] = s;
            step_done[2] = -1;
            board[i][j] = 1;
            i += steps[f][0];
            j += steps[f][1];
            board[i][j] = 2;
            i += steps[s][0];
            j += steps[s][1];
            board[i][j] = 2;
            int find = find_path(board, step_done, n, m, i, j, 2, max_step);
            MPI_Send(step_done, max_step, MPI_INT, 0, find, MPI_COMM_WORLD);
        }
    }
    free(step_done);
    return OK;
}

int main(int argc, char **argv)
{
    int sizes[][2] = {/*{3, 4}, {4, 5}, {5, 5}, {3, 7}, {7, 7},*/ {8, 8}, /*{10, 10}, {3, 14}, {15, 15}, {25, 25}, 
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
        if (myrank == 0)
        {
            fprintf(file, "Solution for %dx%d board:\n", sizes[i][0], sizes[i][1]);
            print_matrix(file, matrix, sizes[i][0], sizes[i][1]);
            fprintf(file, "Time for solution: %lf\n", time / N);
        }
        free_matrix(matrix, sizes[i][0]);
    }

    MPI_Finalize();
    fclose(file);
    return OK;
}
