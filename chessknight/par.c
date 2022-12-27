#include <mpi.h>
#include "lib.h"

#ifdef _OPENMP

#include <omp.h>

#endif

extern int steps[STEPS][2];

int make_nsteps(int ***tasks, int step_count, int n, int m, int i, int j)
{
    if (steps <= 0)
        return 0;

    int **buf = NULL, count = 0, size = 0;
    int pos_i = i, pos_j = j, new_count = 0;
    int step_do[2] = {-1, -1};
    int step = make_step(step_do, 0, n, m, i, j, pos_i, pos_j);
    while (step >= 0)
    {
        if (new_count == size)
        {
            int nsize = size * 2 + 1;
            int **nbuf = alloc_matrix(nsize, 2);
            if (!nbuf)
                return 0;
            for (int s = 0; s < size; s++)
                memcpy(nbuf[s], buf[s], 2 * sizeof(int));
            free_matrix(buf, size);
            buf = nbuf;
            size = nsize;
        }
        step_do[0] = step;
        memcpy(buf[new_count++], step_do, 2 * sizeof(int));
        step = make_step(step_do, 0, n, m, i, j, pos_i, pos_j);
    }
    *tasks = buf;
    count = new_count;
    int task_size = size;
    for (int k = 2; k <= step_count; k++)
    {
        buf = alloc_matrix(size, k + 1);
        if (!buf)
        {
            free_matrix(*tasks, task_size);
            return 0;
        }
        new_count = 0;
        for (int c = 0; c < count; c++)
        {
            int *step_done = (*tasks)[c];
            pos_i = i, pos_j = j;
            for (int p = 0; p < k-1; p++)
            {
                pos_i += steps[step_done[p]][0];
                pos_j += steps[step_done[p]][1];
            }
            step = make_step(step_done, k-1, n, m, i, j, pos_i, pos_j);
            while (step >= 0)
            {
                if (new_count == size)
                {
                    int nsize = size * 2 + 1;
                    int **nbuf = alloc_matrix(nsize, k+1);
                    if (!nbuf)
                    {
                        free_matrix(*tasks, task_size);
                        free_matrix(buf, size);
                        return 0;
                    }
                    for (int s = 0; s < size; s++)
                        memcpy(nbuf[s], buf[s], (k + 1) * sizeof(int));
                    free_matrix(buf, size);
                    buf = nbuf;
                    size = nsize;
                }
                step_done[k-1] = step;
                memcpy(buf[new_count], step_done, k * sizeof(int));
                buf[new_count++][k] = -1;
                step = make_step(step_done, k-1, n, m, i, j, pos_i, pos_j);
            }
        }
        free_matrix(*tasks, task_size);
        *tasks = buf;
        count = new_count;
    }
    for (int c = count; c < size; c++)
        free((*tasks)[c]);
    return count;
}

int solve(int **board, int n, int m, int rank, int nprocs)
{
    if (check_sizes(n, m) != OK)
        return NO_SOLVE;

    int max_step = n * m - 1;
    int n2 = (n + 1) / 2, m2 = (m + 1) / 2, find = FALSE;
    int **tasks = NULL, **step_done = NULL;
    if (rank == 0)
        step_done = alloc_matrix(nprocs, max_step);
    for (int i = 0; i < n2 && !find; i++)
    {
        for (int j = 0; j < m2 && !find; j++)
        {
            if (n * m % 2 && (i + j) % 2)
                continue;

            if (rank == 0)
            {
                int count = make_nsteps(&tasks, FSTEPS, n, m, i, j);
                for (int k = 1; k < nprocs; k++)
                {
                    int kk = count / (nprocs - 1) + count % (nprocs - 1) / k;
                    MPI_Send(&kk, 1, MPI_INT, k, 0, MPI_COMM_WORLD);
                }

                for (int k = 0; k < count; k++)
                    MPI_Send(tasks[k], FSTEPS + 1, MPI_INT, k % (nprocs - 1) + 1, 0, MPI_COMM_WORLD);
                free_matrix(tasks, count);

                for (int k = 1; k < nprocs; k++)
                {
                    MPI_Status status;
                    MPI_Recv(step_done[k], max_step, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
                    if (!find && status.MPI_TAG)
                    {
                        find = status.MPI_TAG;
                        fill_board(board, i, j, step_done[k], max_step);
                    }
                }
                for (int k = 1; k < nprocs; k++)
                    MPI_Send(&find, 1, MPI_INT, k, 0, MPI_COMM_WORLD);
            }
            else 
            {
                int count = 0, where = 0;
                MPI_Status status;
                MPI_Recv(&count, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

                step_done = alloc_matrix(count, max_step);

                for (int k = 0; k < count; k++)
                    MPI_Recv(step_done[k], FSTEPS + 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);

                for (int k = 0; k < count; k++)
                {
                    if (!find)
                    {
                        find = find_path(step_done[k], n, m, i, j, FSTEPS, max_step);
                        if (find)
                            where = k;
                    }
                }
                MPI_Send(step_done[where], max_step, MPI_INT, 0, find, MPI_COMM_WORLD);

                free_matrix(step_done, count);

                MPI_Recv(&find, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
            }
        }
    }
    if (rank == 0)
        free_matrix(step_done, nprocs);
    return OK;
}

int main(int argc, char **argv)
{
    int sizes[][2] = {/*{3, 4}, {4, 5}, {5, 5}, {3, 7}, {7, 7},*/ {8, 8}, /*{10, 10}, {3, 14}, {15, 15}, {25, 25},
                    {39, 39}, {50, 50}, {64, 64}, {73, 73}, {100, 100}, {200, 200}, {500, 500}, {1000, 1000}*/
    };
    int n = sizeof(sizes) / sizeof(sizes[0]);
    FILE *file = stdout;
    if (!file)
        return NO_SOLVE;

    int myrank, nprocs, provided;
    //MPI_Init(&argc, &argv);
    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
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
