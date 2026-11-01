#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <math.h>

#define N 400
#define IDX(i, j) ((i) * (N + 2) + (j))

void init_grid(double *u, int local_n, int rank, int setup);
void write_grid(const double *u, const char *name);

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int start = 1;
    int local_n = N / size + ((rank < N % size) ? 1 : 0);
    if (rank != 0)
    {
        for (int i = 0; i < rank; i++)
        {
            start += N / size + ((i < N % size) ? 1 : 0);
        }
    }

    double *old = malloc((local_n + 2) * (N + 2) * sizeof *old);
    double *new = malloc((local_n + 2) * (N + 2) * sizeof *new);
    if (old == NULL || new == NULL)
    {
        fprintf(stderr, "allocation failed\n");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }

    init_grid(old, local_n, rank, 0);
    init_grid(new, local_n, rank, 0);

    int up = (rank != 0) ? rank - 1 : MPI_PROC_NULL;
    int down = (rank != size - 1) ? rank + 1 : MPI_PROC_NULL;

    double tol = 1e-4;
    int max_iter = 100000;
    int iter = 0;
    double local_max_diff;
    double max_diff;

    double *buf = NULL;
    int counts[size];
    int displs[size];
    double elapsed;

    MPI_Barrier(MPI_COMM_WORLD);
    double t0 = MPI_Wtime();
    do
    {
        local_max_diff = 0.0;
        iter += 1;

        MPI_Sendrecv(&old[IDX(1, 0)], N + 2, MPI_DOUBLE, up, 0, &old[IDX(local_n + 1, 0)], N + 2, MPI_DOUBLE, down, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Sendrecv(&old[IDX(local_n, 0)], N + 2, MPI_DOUBLE, down, 0, &old[IDX(0, 0)], N + 2, MPI_DOUBLE, up, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (int i = 1; i < local_n + 1; i++)
        {
            for (int j = 1; j < N + 1; j++)
            {
                new[IDX(i, j)] = (old[IDX(i - 1, j)] + old[IDX(i + 1, j)] + old[IDX(i, j - 1)] + old[IDX(i, j + 1)]) / 4.0;

                double diff = fabs(new[IDX(i, j)] - old[IDX(i, j)]);
                local_max_diff = local_max_diff < diff ? diff : local_max_diff;
            }
        }
        double *temp = old;
        old = new;
        new = temp;

        MPI_Allreduce(&local_max_diff, &max_diff, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);
    } while (max_diff > tol && iter < max_iter);
    double t1 = MPI_Wtime() - t0;

    MPI_Reduce(&t1, &elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
    if (rank == 0)
    {
        buf = malloc((N + 2) * (N + 2) * sizeof *buf);
        if (buf == NULL)
        {
            fprintf(stderr, "allocation failed\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        init_grid(buf, N, 0, 0);

        for (int i = 0; i < size; i++)
        {
            displs[i] = 1;
            counts[i] = N / size + ((i < N % size) ? 1 : 0);
            counts[i] *= (N + 2);
            if (i != 0)
            {

                for (int j = 0; j < i; j++)
                {
                    displs[i] += N / size + ((j < N % size) ? 1 : 0);
                }
            }
            displs[i] *= (N + 2);
        }
        printf("timing= %fs\n", elapsed);
        printf("iteration count= %d\n", iter);
        printf("final max_diff= %e\n", max_diff);
    }
    MPI_Gatherv(&old[IDX(1, 0)], local_n * (N + 2), MPI_DOUBLE, buf, counts, displs, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (rank == 0)
        write_grid(buf, "mpi_data.txt");

    if (rank == size - 1)
    {
        printf("last row values\n");
        for (int j = 1; j < N; j += 10)
        {
            printf(" %.2f", old[IDX(local_n, j)]);
        }
        printf("\n");
    }

    free(old);
    free(new);
    MPI_Finalize();
    return 0;
}

void init_grid(double *u, int local_n, int rank, int setup)
{
    if (setup == 0)
    {
        for (int i = 0; i < local_n + 2; i++)
        {
            for (int j = 0; j < N + 2; j++)
            {
                u[IDX(i, j)] = 0.0;
            }
        }
        if (rank == 0)
        {
            for (int j = 0; j < N + 2; j++)
            {
                u[IDX(0, j)] = 100.0;
            }
        }
    }
    else
    {
        fprintf(stderr, "Unknown setup %d\n", setup);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
}

void write_grid(const double *u, const char *name)
{
    FILE *snap = fopen(name, "w");
    if (snap == NULL)
    {
        perror(name);
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
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