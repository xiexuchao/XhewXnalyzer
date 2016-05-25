#ifndef _TIMER_H
#define _TIMER_H

#include <time.h>
#include <sys/time.h>

inline long long time_now();
inline long long time_elapsed(long long begin);

#endif