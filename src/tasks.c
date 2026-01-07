#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "tasks.h"
#include <stdbool.h>

static int make_text_payload(const char *s, void **out_data, int *out_size)
{
    int n = (int)strlen(s);
    char *buf = (char *)malloc((long)n + 1);
    if (!buf)
        return -1;
    memcpy(buf, s, (long)n + 1);
    *out_data = buf;
    *out_size = n + 1;
    return 0;
}

static int count_primes(long n, long *out)
{
    if (n < 2)
    {
        *out = 0;
        return 0;
    }

    bool *p = (bool *)malloc(n + 1);
    if (!p)
        return -1;

    memset(p, true, n + 1);
    p[0] = false;
    p[1] = false;

    for (long i = 2; i * i <= n; i++)
    {
        if (p[i])
        {
            for (long k = i * i; k <= n; k += i)
                p[k] = 0;
        }
    }

    long c = 0;
    for (long i = 2; i <= n; i++)
        if (p[i])
            c++;

    free(p);
    *out = c;
    return 0;
}

static int count_divisors(long n, long *out)
{
    if (n < 2)
    {
        *out = 0;
        return 0;
    }

    long x = n;
    long c = 0;

    if (x % 2 == 0)
    {
        c++;
        while (x % 2 == 0)
            x /= 2;
    }

    for (long p = 3; p * p <= x; p += 2)
    {
        if (x % p == 0)
        {
            c++;
            while (x % p == 0)
                x /= p;
        }
    }

    if (x > 1)
        c++;
    *out = c;
    return 0;
}

static int cmp(const void *a, const void *b)
{
    char ca = *(const char *)a;
    char cb = *(const char *)b;
    return ca - cb;
}

static int append_str(char **buf, int *cap, int *len, const char *s)
{
    int add = (int)strlen(s);
    int need = *len + add + 1;
    if (need > *cap)
    {
        int new_cap = (*cap == 0) ? 1024 : *cap;
        while (new_cap < need)
            new_cap *= 2;
        char *nb = (char *)realloc(*buf, (long)new_cap);
        if (!nb)
            return -1;
        *buf = nb;
        *cap = new_cap;
    }
    memcpy(*buf + *len, s, (long)add);
    *len += add;
    (*buf)[*len] = '\0';
    return 0;
}

static int append_line(char **buf, int *cap, int *len, const char *s)
{
    if (append_str(buf, cap, len, s) != 0)
        return -1;
    if (append_str(buf, cap, len, "\n") != 0)
        return -1;
    return 0;
}

static void gen_anagrams_rec(const char *in, int n, int *used, char *cur, int depth,
                             char **out, int *cap, int *len)
{
    if (depth == n)
    {
        cur[n] = ' ';
        append_line(out, cap, len, cur);
        return;
    }

    char last = 0;
    int has_last = 0;

    for (int i = 0; i < n; i++)
    {
        if (used[i])
            continue;
        if (has_last && in[i] == last)
            continue;

        used[i] = 1;
        cur[depth] = in[i];
        gen_anagrams_rec(in, n, used, cur, depth + 1, out, cap, len);
        used[i] = 0;

        last = in[i];
        has_last = 1;
    }
}

static int generate_anagrams_text(const char *name, void **out_data, int *out_size)
{
    int n = (int)strlen(name);
    if (n <= 0)
        return make_text_payload("", out_data, out_size);
    if (n > 8)
        return make_text_payload("ERROR: name too long (max 8)\n", out_data, out_size);

    char sorted[9];
    memcpy(sorted, name, (long)n + 1);
    qsort(sorted, (long)n, sizeof(char), cmp);

    int used[9];
    memset(used, 0, sizeof(used));
    char cur[9];

    char *buf = NULL;
    int cap = 0, len = 0;

    gen_anagrams_rec(sorted, n, used, cur, 0, &buf, &cap, &len);

    if (!buf)
        return make_text_payload("", out_data, out_size);
    *out_data = buf;
    *out_size = len + 1;
    return 0;
}

int execute_job(const Job *job,
                const void *params,
                int params_size,
                void **out_data,
                int *out_size)
{
    *out_data = NULL;
    *out_size = 0;

    if (job->type == CMD_PRIMES || job->type == CMD_PRIMEDIVISORS)
    {
        if (params_size != (int)sizeof(long))
        {
            return make_text_payload("ERROR: bad params\n", out_data, out_size);
        }

        long n = *(const long *)params;
        long ans = 0;
        int rc;

        if (job->type == CMD_PRIMES)
            rc = count_primes(n, &ans);
        else
            rc = count_divisors(n, &ans);

        if (rc != 0)
        {
            return make_text_payload("ERROR: computation failed\n", out_data, out_size);
        }

        long *res = malloc(sizeof(long));
        if (!res)
        {
            return make_text_payload("ERROR: out of memory\n", out_data, out_size);
        }

        *res = ans;
        *out_data = res;
        *out_size = sizeof(long);
        return 0;
    }

    if (job->type == CMD_ANAGRAMS)
    {
        if (params_size <= 0)
            return make_text_payload("ERROR: bad params\n", out_data, out_size);

        return generate_anagrams_text((const char *)params, out_data, out_size);
    }

    return make_text_payload("ERROR: unknown command\n", out_data, out_size);
}