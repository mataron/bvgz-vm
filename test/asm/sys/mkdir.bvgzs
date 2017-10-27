
%data @ok           0x0 /8
%data @path         "test.dir.123"
%data @test         0x0 /8
%data @mkdir_args   0x0 /4

_entry:
    write32     &mkdir_args  0       &path
    syscall 12  &mkdir_args  &ok
    ne          test        ok     0
    jtrue       &on_error   test

    ret

on_error:
    write64     &ok 0          0x1122
    ret
