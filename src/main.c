#include <mpi.h>
#include <stdio.h>
#include "dispatcher.h"
#include "worker.h"

int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (rank == 0) {
        if (argc < 2) {
            MPI_Finalize();
            return 0;
        }
        run_dispatcher(argv[1], size);
    } else {
        run_worker();
    }

    MPI_Finalize();
    return 0;
}
