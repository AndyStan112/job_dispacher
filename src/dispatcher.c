#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>
#include "dispatcher.h"
#include "parser.h"
#include "protocol.h"
#include "common.h"

static void print_and_store_result(int worker, const ResultHeader *hdr)
{
    void *payload = NULL;

    if (hdr->payload_size > 0)
    {
        payload = malloc((size_t)hdr->payload_size);
        MPI_Recv(payload, hdr->payload_size, MPI_BYTE,
                 worker, TAG_RESULT_DATA,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    printf("Job %d client CLI%d: ",
           hdr->job_id, hdr->client_id);

    if (hdr->status == 0)
    {
        if (hdr->payload_size == (int)sizeof(long))
            printf("result=%ld\n", *(long *)payload);
        else
            printf("result=(binary %d bytes)\n", hdr->payload_size);

        FILE *outf;
        char out_filename[64];
        snprintf(out_filename, sizeof(out_filename), "out/CLI%d", hdr->client_id);
        outf = fopen(out_filename, "wb");
        if (outf)
        {
            if (hdr->type == CMD_ANAGRAMS)
            {
                fwrite(payload, 1, (size_t)hdr->payload_size, outf);
            }
            else
            {
                fprintf(outf, "%d", *(long *)payload);
            }
            fclose(outf);
        }
    }
    else
    {
        printf("ERROR: %s\n", payload ? (char *)payload : "");
    }

    free(payload);
}

static void poll_results(int world_size,
                         MPI_Request *requests,
                         ResultHeader *headers,
                         int *idle,
                         int *idle_count)
{
    while (1)
    {
        int idx;
        int flag = 0;

        MPI_Testany(world_size, requests, &idx, &flag, MPI_STATUS_IGNORE);

        if (!flag || idx == MPI_UNDEFINED)
            break;

        print_and_store_result(idx, &headers[idx]);
        requests[idx] = MPI_REQUEST_NULL;
        idle[(*idle_count)++] = idx;
    }
}

void run_dispatcher(const char *filename, int world_size)
{
    FILE *f = fopen(filename, "r");
    if (!f)
        return;

    int max_workers = world_size - 1;

    int *idle = malloc((size_t)max_workers * sizeof(int));
    int idle_count = 0;

    MPI_Request *requests = malloc((size_t)world_size * sizeof(MPI_Request));
    ResultHeader *headers = malloc((size_t)world_size * sizeof(ResultHeader));

    for (int i = 0; i < world_size; i++)
        requests[i] = MPI_REQUEST_NULL;

    for (int i = 1; i < world_size; i++)
        idle[idle_count++] = i;

    int job_id = 0;
    char line[MAX_LINE];

    while (fgets(line, sizeof(line), f))
    {
        poll_results(world_size, requests, headers, idle, &idle_count);

        ParsedJob pj;
        int wait_seconds;

        ParseResult r = parse_line(line, &pj, &wait_seconds);
        if (r == PARSE_INVALID)
            continue;

        if (r == PARSE_WAIT)
        {
            int ticks = wait_seconds * 10;
            for (int i = 0; i < ticks; i++)
            {
                usleep(100000);
                poll_results(world_size, requests, headers, idle, &idle_count);
            }
            continue;
        }

        while (idle_count == 0)
        {
            int idx;
            MPI_Waitany(world_size, requests, &idx, MPI_STATUS_IGNORE);

            if (idx == MPI_UNDEFINED)
                continue;

            print_and_store_result(idx, &headers[idx]);
            requests[idx] = MPI_REQUEST_NULL;
            idle[idle_count++] = idx;

            poll_results(world_size, requests, headers, idle, &idle_count);
        }

        int worker = idle[--idle_count];

        pj.job.job_id = job_id++;
        pj.job.t_received = MPI_Wtime();

        MPI_Send(&pj.job, sizeof(Job), MPI_BYTE,
                 worker, TAG_JOB_HEADER, MPI_COMM_WORLD);

        pj.job.t_dispatched = MPI_Wtime();

        MPI_Send(pj.params.data, pj.params.size, MPI_BYTE,
                 worker, TAG_JOB_PARAMS, MPI_COMM_WORLD);

        free_parsed_job(&pj);

        MPI_Irecv(&headers[worker], sizeof(ResultHeader), MPI_BYTE,
                  worker, TAG_RESULT_HEADER,
                  MPI_COMM_WORLD, &requests[worker]);

        poll_results(world_size, requests, headers, idle, &idle_count);
    }

    fclose(f);

    poll_results(world_size, requests, headers, idle, &idle_count);

    while (1)
    {
        int any_pending = 0;
        for (int w = 1; w < world_size; w++)
        {
            if (requests[w] != MPI_REQUEST_NULL)
            {
                any_pending = 1;
                break;
            }
        }
        if (!any_pending)
            break;

        int idx;
        MPI_Waitany(world_size, requests, &idx, MPI_STATUS_IGNORE);

        if (idx == MPI_UNDEFINED)
            continue;

        print_and_store_result(idx, &headers[idx]);
        requests[idx] = MPI_REQUEST_NULL;

        poll_results(world_size, requests, headers, idle, &idle_count);
    }

    for (int i = 1; i < world_size; i++)
        MPI_Send(NULL, 0, MPI_BYTE, i, TAG_TERMINATE, MPI_COMM_WORLD);

    free(idle);
    free(requests);
    free(headers);
}
