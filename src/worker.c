#include <mpi.h>
#include <stdlib.h>
#include "worker.h"
#include "protocol.h"
#include "common.h"
#include "tasks.h"

void run_worker(void)
{
    while (1)
    {
        MPI_Status status;
        MPI_Probe(0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        if (status.MPI_TAG == TAG_TERMINATE)
        {
            MPI_Recv(NULL, 0, MPI_BYTE, 0, TAG_TERMINATE, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            break;
        }

        Job job;
        MPI_Recv(&job, sizeof(Job), MPI_BYTE, 0, TAG_JOB_HEADER, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        int params_size = 0;
        if (job.type == CMD_PRIMES || job.type == CMD_PRIMEDIVISORS)
            params_size = (int)sizeof(long);
        else if (job.type == CMD_ANAGRAMS)
            params_size = MAX_NAME;

        void *params = malloc(params_size);
        MPI_Recv(params, params_size, MPI_BYTE, 0, TAG_JOB_PARAMS, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        void *out = NULL;
        int out_size = 0;
        int rc = execute_job(&job, params, params_size, &out, &out_size);

        free(params);

        ResultHeader hdr;
        hdr.job_id = job.job_id;
        hdr.client_id = job.client_id;
        hdr.status = (rc == 0) ? 0 : 1;
        hdr.payload_size = out_size;

        MPI_Send(&hdr, sizeof(ResultHeader), MPI_BYTE, 0, TAG_RESULT_HEADER, MPI_COMM_WORLD);
        if (out_size > 0)
            MPI_Send(out, out_size, MPI_BYTE, 0, TAG_RESULT_DATA, MPI_COMM_WORLD);

        free(out);
    }
}
