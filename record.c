#include <stdio.h>
#include <string.h>

#include "fibdrv.h"

unsigned long long exec_cnt[MAX_LENGTH];
double exec_avg[MAX_LENGTH];
double exec_var[MAX_LENGTH];

void record_init()
{
    memset(exec_avg, 0, sizeof(exec_avg));
    memset(exec_var, 0, sizeof(exec_var));
    memset(exec_cnt, 0, sizeof(exec_cnt));
}


void record(long long k, unsigned long long t)
{
    if (k >= MAX_LENGTH)
        return;

    if (t >= 1200) {
        printf("k=%lld t=%llu, drop\n", k, t);
        return;
    }

    double x = (double) t;
    double delta = x - exec_avg[k];

    exec_cnt[k]++;
    exec_avg[k] += delta / exec_cnt[k];
    exec_var[k] += delta * (x - exec_avg[k]);
}

void finalize_var()
{
    for (size_t i = 0; i < MAX_LENGTH; i++)
        exec_var[i] = exec_var[i] / (exec_cnt[i] - 1);
}

void record_show(FILE *fp)
{
    finalize_var();
    for (size_t i = 0; i < MAX_LENGTH; i++) {
        if (exec_cnt[i] <= 100) {
            printf("insufficient measurement for k=%zu\n", i);
            continue;
        }
        fprintf(fp, "%zu , %llu: %lf, %lf \n", i, exec_cnt[i], exec_avg[i],
                exec_var[i]);
    }
}