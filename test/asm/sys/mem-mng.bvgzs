
%data @ok           0x1122334455667788 /8
%data @err          0x0 /8
%data @test         0x0 /8
%data @args         0x0 /8


_entry:
    ; ensure current memory size
    syscall 23  0           &err
    ne          test        err     32
    jtrue       &on_error   test

    ; expand 40 bytes
    write64     &args       0      40
    syscall 21  &args       &err
    ne          test        err     0
    jtrue       &on_error   test

    ; ensure current memory size
    syscall 23  0           &err
    ne          test        err     72
    jtrue       &on_error   test

    ; retract 20 bytes
    write64     &args       0       20
    syscall 22  &args       &err
    ne          test        err     0
    jtrue       &on_error   test

    ; ensure current memory size
    syscall 23  0           &err
    ne          test        err     52
    jtrue       &on_error   test

    ; done!
    write64     &ok 0         0
    ret

on_error:
    write64     &ok 0          0x1122
    ret
