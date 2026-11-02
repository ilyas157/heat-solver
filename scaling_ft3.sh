#!/bin/bash
#SBATCH -J heat_scaling
#SBATCH -o %x-%J.out
#SBATCH -e %x-%J.error
#SBATCH -n 64
#SBATCH --time=0-00:30:00
#SBATCH --mem-per-cpu 1G

module load intel impi
export I_MPI_DEBUG=0

for r in 1 2 3; do
    for p in 1 2 4 8 16 32 64; do
        echo "run=$r ranks=$p"
        mpirun -np $p ./heat_mpi 4000 0 1000
    done
done
