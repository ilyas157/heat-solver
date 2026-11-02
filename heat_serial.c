#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define IDX(i, j, n) ((i) * ((n) + 2) + (j))

void init_grid(double *u, int setup, int N);
void write_grid(const double *u, const char *name, int N);

int main(int argc, char **argv)
{
    // defaults
    int N = 100;
    int save_snaps = 1;

    if (argc > 1)
        N = atoi(argv[1]);
    if (argc > 2)
        save_snaps = atoi(argv[2]);

    if (N <= 0 || (save_snaps != 0 && save_snaps != 1))
    {
        fprintf(stderr, "usage: %s N save_snaps(0 or 1)\n", argv[0]);
        return EXIT_FAILURE;
    }
    double *old = malloc((N + 2) * (N + 2) * sizeof *old);
    double *new = malloc((N + 2) * (N + 2) * sizeof *new);
    if (old == NULL || new == NULL)
    {
        fprintf(stderr, "allocation failed\n");
        return EXIT_FAILURE;
    }

    init_grid(old, 0, N);
    init_grid(new, 0, N);

    double tol = 1e-4;
    int max_iter = 100000;
    int iter = 0;
    double max_diff;
    int count = 0;
    char name[64];

    if (save_snaps == 1)
    {

        sprintf(name, "./snaps/snap_%04d.txt", count);
        write_grid(old, name, N);
        count++;
    }

    clock_t start = clock();
    do
    {
        max_diff = 0.0;
        iter += 1;
        for (int i = 1; i < N + 1; i++)
        {
            for (int j = 1; j < N + 1; j++)
            {
                new[IDX(i, j, N)] = (old[IDX(i - 1, j, N)] + old[IDX(i + 1, j, N)] + old[IDX(i, j - 1, N)] + old[IDX(i, j + 1, N)]) / 4.0;

                double diff = fabs(new[IDX(i, j, N)] - old[IDX(i, j, N)]);
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
        if (save_snaps == 1)
        {
            if (iter % 200 == 0)
            {
                sprintf(name, "./snaps/snap_%04d.txt", count);
                write_grid(old, name, N);
                count++;
            }
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
        printf(" %.2f", old[IDX(N, j, N)]);
    }
    printf("\n");

    if (save_snaps == 1)
    {
        sprintf(name, "./snaps/snap_%04d.txt", count);
        write_grid(old, name, N);
    }
    write_grid(old, "data.txt", N);

    free(old);
    free(new);
    return 0;
}

void init_grid(double *u, int setup, int N)
{
    if (setup == 0)
    {
        // setting the top row to 100
        for (int j = 0; j < N + 2; j++)
        {
            u[IDX(0, j, N)] = 100.0;
        }
        // everything below to 0
        for (int i = 1; i < N + 2; i++)
        {
            for (int j = 0; j < N + 2; j++)
            {
                u[IDX(i, j, N)] = 0.0;
            }
        }
    }
    else
    {
        fprintf(stderr, "Unknown setup %d\n", setup);
        exit(EXIT_FAILURE);
    }
}

void write_grid(const double *u, const char *name, int N)
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
            fprintf(snap, "%f ", u[IDX(i, j, N)]);
        }
        fprintf(snap, "\n");
    }
    fclose(snap);
}