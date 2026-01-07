#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

ParseResult parse_line(const char *line, ParsedJob *out, int *wait_seconds)
{
    if (sscanf(line, "WAIT %d", wait_seconds) == 1)
        return PARSE_WAIT;

    char client[32], cmd[32];
    if (sscanf(line, "%s %s", client, cmd) != 2)
        return PARSE_INVALID;

    out->job.client_id = atoi(client + 3);

    if (strcmp(cmd, "PRIMES") == 0 || strcmp(cmd, "PRIMEDIVISORS") == 0)
    {
        long *n = malloc(sizeof(long));
        sscanf(line, "%*s %*s %ld", n);

        out->job.type = (cmd[5] == 'S') ? CMD_PRIMES : CMD_PRIMEDIVISORS;
        out->params.data = n;
        out->params.size = sizeof(long);
        return PARSE_JOB;
    }

    if (strcmp(cmd, "ANAGRAMS") == 0)
    {
        char *name = malloc(MAX_NAME);
        sscanf(line, "%*s %*s %s", name);

        out->job.type = CMD_ANAGRAMS;
        out->params.data = name;
        out->params.size = MAX_NAME;
        return PARSE_JOB;
    }

    return PARSE_INVALID;
}

void free_parsed_job(ParsedJob *job)
{
    free(job->params.data);
}
