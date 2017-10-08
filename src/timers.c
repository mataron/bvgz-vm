#include <stdlib.h>
#include <errno.h>

#include "vm.h"
#include "timers.h"


uint32_t fire_timer_events(vm_t* vm)
{
    uint32_t events = 0;

    struct timespec now = { 0, 0 };
    if (clock_gettime(CLOCK_REALTIME, &now) < 0)
    {
        vm->error_no = errno;
    }

    uint32_t n_timers = vm->n_timers;
    for (uint32_t i = 0; i < n_timers; i++)
    {
        vm_timer_t* tmr = vm->timers + i;
        if (TIMESPEC_GREATER_EQUAL(now, tmr->expires_at))
        {
            events++;
            make_procedure(tmr->iptr, vm);
            if (i + 1 < n_timers)
            {
                *tmr = vm->timers[n_timers - 1];
            }
            n_timers--;
            i--;
        }
    }

    if (n_timers != vm->n_timers)
    {
        vm->timers = realloc(vm->timers, n_timers * sizeof(vm_timer_t));
        vm->n_timers = n_timers;
    }

    return events;
}


int has_pending_timer_events(vm_t* vm)
{
    return vm->n_timers > 0;
}
