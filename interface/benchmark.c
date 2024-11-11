#include "benchmark.h"

#include <math.h>
#include <sys/time.h>

size_t ubenchmark(const voidfunc_t func, const size_t run_amount, const double time_interval){
    struct timespec start, end, max;
    size_t ran_amount = 0;

    clock_gettime(CLOCK_MONOTONIC, &start);

    max.tv_sec = start.tv_sec + (time_t)floor(time_interval);
    max.tv_nsec = start.tv_nsec + (long)((time_interval - floor(time_interval)) * 1e9);

    if (max.tv_nsec >= 1e9) {
        max.tv_nsec -= 1e9;
        max.tv_sec++;
    }

    while (1) {
        for (int i = 0; i < run_amount; i++) func();
        ran_amount += run_amount;

        clock_gettime(CLOCK_MONOTONIC, &end);

        if (end.tv_sec > max.tv_sec || (end.tv_sec == max.tv_sec && end.tv_nsec >= max.tv_nsec)) break;
    }
    return (((end.tv_sec - start.tv_sec) * 1e6) + ((end.tv_nsec - start.tv_nsec) / 1e3))/ ran_amount;
}
double benchmark(const voidfunc_t func, const size_t run_amount, const double time_interval) {
    return ubenchmark(func, run_amount, time_interval) / 1.0e6;
}