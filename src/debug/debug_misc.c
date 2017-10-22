#include "debug_util.h"
#include "instns/instn.h"
#include "bytecode.h"


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


int dbg_disasm(int argc, char** argv, dbg_state_t* state)
{
    instn_t instn;
    uint32_t begin = 0;
    uint32_t end = state->vm->codesz;

    if (argc > 3)
    {
        printf("Usage: %s ([label|address]) (offset)\n", argv[0]);
        return 0;
    }

    if (argc > 1)
    {
        begin = resolve_code_address(argv[1], state);
        if (begin == (uint32_t)-1)
        {
            printf("bad code address: %s\n", argv[1]);
            return 0;
        }
    }

    if (argc > 2)
    {
        uint32_t offset = parse_uint(argv[2], 10);
        if (offset == (uint32_t)-1)
        {
            printf("bad offset: %s\n", argv[2]);
            return 0;
        }

        end = begin + offset;
    }

    for (uint32_t addr = begin; addr < end; )
    {
        unsigned exceptions = state->vm->exceptions;
        int32_t offt = decode_instn(state->vm->code + addr,
            state->vm, &instn);
        if (offt < 0)
        {
            printf("decode failed at: 0x%08x\n", addr);
            unsigned x = state->vm->exceptions & ~exceptions;
            print_exception("  ", x);
            state->vm->exceptions = x;
            return 0;
        }

        int r = print_code_address(addr, state, 0);
        if (r)
        {
            printf(":\n");
            print_code_address(addr, state, 1);
        }
        printf(":  ");
        print_instn(&instn, state);
        printf("\n");

        addr += offt;
    }

    return 0;
}
