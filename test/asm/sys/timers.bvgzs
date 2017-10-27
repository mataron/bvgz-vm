
%data @ok 0x0 /8
%data @sysret 0x0 /8
%data @test 0x0 /8
%data @begin_t 0x0 /16
%data @tmout_t 0x0 /16
%data @timeout_args 0x0 /12
%data @ptr 0x0 /4
%data @tmp 0x0 /8

_entry:
    syscall 2 &begin_t &sysret
    ne test sysret 0
    jtrue &on_error test

    write32 &timeout_args 0 &callback
    write32 &ptr 0 &timeout_args
    write64 ptr 4 250
    syscall 3 &timeout_args &sysret
    ne test sysret 0
    jtrue &on_error test

    ret

on_error:
    write64 &ok 0 0xdeadbeaf
    ret

callback:
    syscall 2 &tmout_t &sysret
    ne test sysret 0
    jtrue &on_error test

    ; ensure time delta is <= 1s
    eq ok begin_t tmout_t
    jtrue &done ok
    write64 &tmp 0 begin_t
    add tmp 1
    eq ok tmp tmout_t
    jmp &done

done:
    ret
