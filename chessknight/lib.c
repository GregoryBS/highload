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

/* 
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
*/

int make_step(int **board, int n, int m, int pos_i, int pos_j, int done)
{
    int last = sizeof(steps) / sizeof(steps[0]);
    if (++done == last)
        return NO_SOLVE;

    for (int i = done; i < last; i++)
    {
        int buf_i = pos_i + steps[i][0];
        int buf_j = pos_j + steps[i][1];
        if (buf_i < 0 || buf_i >= n || buf_j < 0 || buf_j >= m || board[buf_i][buf_j] > 0)
            continue;
        return i;
    }
    return NO_SOLVE;
}

int find_path(int **board, int *step_done, int n, int m, int pos_i, int pos_j, int step, int max_step)
{
    int find = FALSE, step_index = 0;
    while (step < max_step)
    {
        step_index = make_step(board, n, m, pos_i, pos_j, step_done[step]);
        if (step_index >= 0)
        {
            step_done[step] = step_index;
            board[pos_i][pos_j] = ++step;
            pos_i += steps[step_index][0];
            pos_j += steps[step_index][1];
            step_done[step] = -1;
        }
        else if (step > 0)
        {
            board[pos_i][pos_j] = 0;
            step--;
            pos_i -= steps[step_done[step]][0];
            pos_j -= steps[step_done[step]][1];
        }
        else
            break;
    }
    if (step >= max_step)
    {
        board[pos_i][pos_j] = ++step;
        find = TRUE;
    }
    return find;
}

int check_sizes(int n, int m) 
{
    if (n > m)
    {
        m += n;
        n = m - n;
        m -= n;
    }
    if (!((n == 1 && m == 1) || (n == 3 && (m == 4 || m > 6)) || (n >= 4 && m >= 5)))
        return NO_SOLVE;
    return OK;
}