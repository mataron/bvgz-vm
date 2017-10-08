
%data @ok 0x1122334455667788 /8
%data @fd 0x0 /8
%data @path "Makefile"
%data @open_args    0x0 /12
%data @test  0x0 /8

_entry:
    cp32        open_args   &path
    write64     &open_args  4  0x0

    syscall 8   &open_args  &fd
    eq          test        fd 0
    jtrue       &on_error   test

    syscall 5   &fd         &ok
    ret

on_error:
    cp64        ok          0x1122
    ret
