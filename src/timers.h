#ifndef _BVGZ_TIMERS_H
#define _BVGZ_TIMERS_H

#include <stdint.h>
#include <time.h>


typedef struct _vm_timer_t
{
    struct timespec expires_at;
    uint32_t iptr;
}
vm_timer_t;


#define TIMESPEC_GREATER_EQUAL(a, b) \
    ((a).tv_sec > (b).tv_sec || \
        ((a).tv_sec == (b).tv_sec && (a).tv_nsec >= (b).tv_nsec))


#define MILLISECONDS_TO_SECONDS(ms)     ((ms) / 1000)
#define MILLISECONDS_TO_NANOSECONDS(ms) (1000000 * ((ms) % 1000))


#endif
