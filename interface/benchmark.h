#pragma once
#include <stddef.h>
#include <time.h>


typedef void (*voidfunc_t)();



// Run a function many times benchmarking it. Error +/- 0.2%
//
// Param func: Function to be called
// Param run_amount: Percision amount. Bigger = closer to actual fuction runtime, but may go beyond time_interval
// Param time_interval: Amount of time to run the function repeatedly for
//
// Return: Average microseconds per call
size_t ubenchmark(voidfunc_t func, size_t run_amount, double time_interval);

// Same as ubenchmark() but with secounds instead of microsecounds
double benchmark(voidfunc_t func, size_t run_amount, double time_interval);