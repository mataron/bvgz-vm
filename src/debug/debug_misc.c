#include "debug_util.h"


int dbg_io(int argc, char** argv, dbg_state_t* state)
{
    if (!(state->flags & F_RUNNING))
    {
        printf("program is not running!\n");
        return 0;
    }

    printf("open fds: %u   (total event handlers: %u)\n",
        state->vm->io.used_fds, state->vm->io.n_io_events);

    for (uint32_t i = 0, index = 0; i < state->vm->io.n_fds; i++)
    {
        vm_fd_t* fd = state->vm->io.fds + i;
        if (fd->used)
        {
            printf("fd[%u]: fd=%d  events:%u\n",
                index, fd->fd, fd->n_events);
            index++;
        }
    }
    return 0;
}


int dbg_children(int argc, char** argv, dbg_state_t* state)
{
    if (!(state->flags & F_RUNNING))
    {
        printf("program is not running!\n");
        return 0;
    }

    printf("child processes: %u\n", state->vm->proc.n_proc);

    for (uint32_t p = 0, index = 0; p < state->vm->proc.n_proc; p++)
    {
        vm_child_t* ch = state->vm->proc.child_proc + p;
        if (ch->used)
        {
            printf("child[%u]: pid=%d  exit c/b:%u\n",
                index, ch->pid, ch->n_exit_cb);
            index++;
        }
    }
    return 0;
}
