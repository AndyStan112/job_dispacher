#ifndef PARSER_H
#define PARSER_H

#include "common.h"

typedef enum {
    PARSE_JOB,
    PARSE_WAIT,
    PARSE_INVALID
} ParseResult;

typedef struct {
    void *data;
    int size;
} JobParams;

typedef struct {
    Job job;
    JobParams params;
} ParsedJob;

ParseResult parse_line(const char *line, ParsedJob *out, int *wait_seconds);
void free_parsed_job(ParsedJob *job);

#endif
