#include <stdio.h>
#include <unistd.h>
#include <mpi.h>
#include "dispatcher.h"
#include "parser.h"
#include "protocol.h"

void run_dispatcher(const char *filename, int world_size)
{
    FILE *f = fopen(filename, "r");
    if (!f) return;

    int next_worker = 1;
    int job_id = 0;
    char line[MAX_LINE];

    while (fgets(line, sizeof(line), f)) {
        ParsedJob pj;
        int wait_seconds;

        ParseResult r = parse_line(line, &pj, &wait_seconds);
        if (r == PARSE_INVALID)
            continue;

        if (r == PARSE_WAIT) {
            sleep(wait_seconds);
            continue;
        }

        pj.job.job_id = job_id++;
        pj.job.t_received = MPI_Wtime();

        MPI_Send(&pj.job, sizeof(Job), MPI_BYTE,
                 next_worker, TAG_JOB_HEADER, MPI_COMM_WORLD);

        pj.job.t_dispatched = MPI_Wtime();

        MPI_Send(pj.params.data, pj.params.size, MPI_BYTE,
                 next_worker, TAG_JOB_PARAMS, MPI_COMM_WORLD);

        free_parsed_job(&pj);

        Result res;
        MPI_Recv(&res, sizeof(Result), MPI_BYTE,
                 MPI_ANY_SOURCE, TAG_RESULT,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        printf("Job %d finished for client CLI%d\n",
               res.job_id, res.client_id);

        next_worker = (next_worker % (world_size - 1)) + 1;
    }

    fclose(f);

    for (int i = 1; i < world_size; i++)
        MPI_Send(NULL, 0, MPI_BYTE, i, TAG_TERMINATE, MPI_COMM_WORLD);
}
