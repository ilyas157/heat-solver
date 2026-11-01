#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define N 100
#define IDX(i, j) ((i) * (N + 2) + (j))

void init_grid(double *u, int setup);
void write_grid(const double *u, const char *name);

int main(int argc, char **argv)
{
    double *old = malloc((N + 2) * (N + 2) * sizeof *old);
    double *new = malloc((N + 2) * (N + 2) * sizeof *new);
    if (old == NULL || new == NULL)
    {
        fprintf(stderr, "allocation failed\n");
        return EXIT_FAILURE;
    }

    init_grid(old, 0);
    init_grid(new, 0);

    double tol = 1e-4;
    int max_iter = 100000;
    int iter = 0;
    double max_diff;
    int count = 0;
    char name[64];

    sprintf(name, "./snaps/snap_%04d.txt", count);
    write_grid(old, name);
    count++;

    clock_t start = clock();
    do
    {
        max_diff = 0.0;
        iter += 1;
        for (int i = 1; i < N + 1; i++)
        {
            for (int j = 1; j < N + 1; j++)
            {
                new[IDX(i, j)] = (old[IDX(i - 1, j)] + old[IDX(i + 1, j)] + old[IDX(i, j - 1)] + old[IDX(i, j + 1)]) / 4.0;

                double diff = fabs(new[IDX(i, j)] - old[IDX(i, j)]);
                max_diff = max_diff < diff ? diff : max_diff;
            }
        }
        if (iter % 10000 == 0)
        {
            printf("iter %d max_diff= %e\n", iter, max_diff);
        }

        double *temp = old;
        old = new;
        new = temp;

        // snapshots
        if (iter % 200 == 0)
        {
            sprintf(name, "./snaps/snap_%04d.txt", count);
            write_grid(old, name);
            count++;
        }

    } while (max_diff > tol && iter < max_iter);
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;

    printf("timing= %fs\n", elapsed);
    printf("iteration count= %d\n", iter);
    printf("final max_diff= %e\n", max_diff);

    printf("some row values\n");
    for (int j = 1; j < N; j += 10)
    {
        printf(" %.2f", old[IDX(N, j)]);
    }
    printf("\n");

    sprintf(name, "./snaps/snap_%04d.txt", count);
    write_grid(old, name);
    write_grid(old, "data.txt");

    free(old);
    free(new);
    return 0;
}

void init_grid(double *u, int setup)
{
    if (setup == 0)
    {
        // setting the top row to 100
        for (int j = 0; j < N + 2; j++)
        {
            u[IDX(0, j)] = 100.0;
        }
        // everything below to 0
        for (int i = 1; i < N + 2; i++)
        {
            for (int j = 0; j < N + 2; j++)
            {
                u[IDX(i, j)] = 0.0;
            }
        }
    }
    else
    {
        fprintf(stderr, "Unknown setup %d\n", setup);
        exit(EXIT_FAILURE);
    }
}

void write_grid(const double *u, const char *name)
{
    FILE *snap = fopen(name, "w");
    if (snap == NULL)
    {
        perror(name);
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < N + 2; i++)
    {
        for (int j = 0; j < N + 2; j++)
        {
            fprintf(snap, "%f ", u[IDX(i, j)]);
        }
        fprintf(snap, "\n");
    }
    fclose(snap);
}