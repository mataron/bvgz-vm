
%data @ok           0x0 /8
%data @struct       0x0 /128
%data @path         "Makefile"
%data @test         0x0 /8
%data @stat_args    0x0 /8

_entry:
    write32     &stat_args  0       &path
    write32     &stat_args  4       &struct
    syscall 9   &stat_args  &ok
    ne          test        ok     0
    jtrue       &on_error   test

    ret

on_error:
    write64     &ok 0          0x1122
    ret
