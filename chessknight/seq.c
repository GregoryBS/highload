#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define TRUE 1
#define FALSE 0
#define N 10

#define OK 0
#define NO_SOLVE -1

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

int steps[][2] = {{-1, -2}, {-2, -1}, {-2, 1}, {1, -2}, {-1, 2}, {2, -1}, {1, 2}, {2, 1}};

int make_step(int **board, int n, int m, int pos_i, int pos_j, int done)
{
    int k = sizeof(steps) / sizeof(steps[0]);
    int moves[k], index[k], count = 0;
    int buf_i, buf_j, next_i, next_j;
    for (int i = 0; i < k; i++)
    {
        moves[i] = 0;
        index[i] = 0;
        buf_i = pos_i + steps[i][0];
        buf_j = pos_j + steps[i][1];
        if (buf_i < 0 || buf_i >= n || buf_j < 0 || buf_j >= m || board[buf_i][buf_j] > 0)
            continue;

        int buf = 0;
        for (int j = 0; j < k; j++)
        {
            next_i = buf_i + steps[j][0];
            next_j = buf_j + steps[j][1];
            if (next_i >= 0 && next_i < n && next_j >= 0 && next_j < m && board[next_i][next_j] == 0)
                buf++;
        }
        next_j = 0;
        for (int j = 0; j < count; j++)
            if (buf >= moves[j])
                next_j++;
        for (int j = count; j > next_j; j--)
        {
            moves[j] = moves[j - 1];
            index[j] = index[j - 1];
        }
        moves[next_j] = buf;
        index[next_j] = i;
        count++;
    }
    if (count == 0 || index[count - 1] == done)
        return NO_SOLVE;
    if (done < 0)
        return index[0];
    for (int i = 0; i < count; i++)
        if (index[i] == done)
            return index[i + 1];
}

int find_path(int **board, int n, int m, int pos_i, int pos_j)
{
    int find = FALSE, max_step = n * m, step = 1, step_index = 0;
    int **step_done = alloc_matrix(n, m);
    board[pos_i][pos_j] = 1;
    step_done[pos_i][pos_j] = -1;
    while (step < max_step)
    {
        step_index = make_step(board, n, m, pos_i, pos_j, step_done[pos_i][pos_j]);
        if (step_index >= 0)
        {
            step_done[pos_i][pos_j] = step_index;
            pos_i += steps[step_index][0];
            pos_j += steps[step_index][1];
            board[pos_i][pos_j] = ++step;
            step_done[pos_i][pos_j] = -1;
        }
        else if (step > 0)
        {
            board[pos_i][pos_j] = 0;
            pos_i -= steps[step_done[pos_i][pos_j]][0];
            pos_j -= steps[step_done[pos_i][pos_j]][1];
            step--;
        }
        else
            break;
    }
    if (step == max_step)
        find = TRUE;
    free_matrix(step_done, n);
    return find;
}

int solve(int **board, int n, int m)
{
    // if (n > m)
    // {
    //     m += n;
    //     n = m - n;
    //     m -= n;
    // }
    if (!((n == 1 && m == 1) || (n == 3 && (m == 4 || m > 6)) || (n >= 4 && m >= 5)))
        return NO_SOLVE;

    int n2 = (n + 1) / 2, m2 = (m + 1) / 2, flag = TRUE;
    for (int i = 0; i < n2 && flag; i++)
    {
        for (int j = 0; j < m2 && flag; j++)
        {
            if (n * m % 2 && (i + j) % 2)
                continue;

            if (find_path(board, n, m, i, j))
                flag = FALSE;
            else
                zero_matrix(board, n, m);
        }
    }
    return OK;
}

int main(int argc, char **argv)
{
    int sizes[][2] = {{3, 4}, {4, 5}, {5, 5}, {3, 7}, {7, 7}, {8, 8}, {10, 10}, {3, 14}, {15, 15}, {25, 25}, 
                    {39, 39}, {50, 50}, {64, 64}, {73, 73}, {100, 100}, {200, 200}, {500, 500}, {1000, 1000}};
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
