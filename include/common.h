#ifndef COMMON_H
#define COMMON_H

#include <mpi.h>

#define MAX_LINE 256
#define MAX_NAME 16

typedef enum
{
    CMD_PRIMES,
    CMD_PRIMEDIVISORS,
    CMD_ANAGRAMS
} CommandType;

typedef struct
{
    int job_id;
    int client_id;
    CommandType type;
    double t_received;
    double t_dispatched;
} Job;

typedef struct
{
    int job_id;
    int client_id;
    CommandType type;
    int status;
    int payload_size;
} ResultHeader;

#endif
