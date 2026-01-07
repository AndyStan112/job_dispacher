#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>
#include "dispatcher.h"
#include "parser.h"
#include "protocol.h"
#include "common.h"

void run_dispatcher(const char *filename, int world_size)
{
    FILE *f = fopen(filename, "r");
    if (!f)
        return;

    int next_worker = 1;
    int job_id = 0;
    char line[MAX_LINE];

    while (fgets(line, sizeof(line), f))
    {
        ParsedJob pj;
        int wait_seconds;

        ParseResult r = parse_line(line, &pj, &wait_seconds);
        if (r == PARSE_INVALID)
            continue;

        if (r == PARSE_WAIT)
        {
            sleep(wait_seconds);
            continue;
        }

        pj.job.job_id = job_id++;
        pj.job.t_received = MPI_Wtime();

        MPI_Send(&pj.job, sizeof(Job), MPI_BYTE, next_worker, TAG_JOB_HEADER, MPI_COMM_WORLD);
        pj.job.t_dispatched = MPI_Wtime();

        MPI_Send(pj.params.data, pj.params.size, MPI_BYTE, next_worker, TAG_JOB_PARAMS, MPI_COMM_WORLD);
        free_parsed_job(&pj);

        ResultHeader hdr;
        MPI_Recv(&hdr, sizeof(ResultHeader), MPI_BYTE,
                 MPI_ANY_SOURCE, TAG_RESULT_HEADER,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        void *payload = NULL;
        if (hdr.payload_size > 0)
        {
            payload = malloc((size_t)hdr.payload_size);
            MPI_Recv(payload, hdr.payload_size, MPI_BYTE,
                     MPI_ANY_SOURCE, TAG_RESULT_DATA,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }

        printf("Job %d client CLI%d: ",
               hdr.job_id, hdr.client_id);

        if (hdr.status == 0)
        {
            if (hdr.payload_size == sizeof(long))
            {
                long value = *(long *)payload;
                printf("result=%ld\n", value);
            }
            else
            {
                printf("result=(binary %d bytes)\n", hdr.payload_size);
            }
        }
        else
        {
            printf("ERROR: %s\n", payload ? (char *)payload : "");
        }

        free(payload);

        next_worker = (next_worker % (world_size - 1)) + 1;
    }

    fclose(f);

    for (int i = 1; i < world_size; i++)
        MPI_Send(NULL, 0, MPI_BYTE, i, TAG_TERMINATE, MPI_COMM_WORLD);
}
