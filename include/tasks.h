#ifndef TASKS_H
#define TASKS_H

#include "common.h"

int execute_job(const Job *job, const void *params, int params_size, void **out_data, int *out_size);

#endif
