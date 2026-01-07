#include <mpi.h>
#include <stdlib.h>
#include "worker.h"
#include "protocol.h"
#include "common.h"

void run_worker(void)
{
    while (1) {
        MPI_Status status;
        MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if (status.MPI_TAG == TAG_TERMINATE) {
            MPI_Recv(NULL, 0, MPI_BYTE, 0,
                     TAG_TERMINATE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            break;
        }

        Job job;
        MPI_Recv(&job, sizeof(Job), MPI_BYTE, 0,
                 TAG_JOB_HEADER, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        void *params = NULL;
        int size = 0;

        if (job.type == CMD_PRIMES || job.type == CMD_PRIMEDIVISORS)
            size = sizeof(long);
        else if (job.type == CMD_ANAGRAMS)
            size = MAX_NAME;

        params = malloc(size);
        MPI_Recv(params, size, MPI_BYTE, 0,
                 TAG_JOB_PARAMS, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        free(params);

        Result res = { job.job_id, job.client_id };
        MPI_Send(&res, sizeof(Result), MPI_BYTE, 0,
                 TAG_RESULT, MPI_COMM_WORLD);
    }
}
