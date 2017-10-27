
%data @ok           0x0 /8
%data @path         "test.file.123"
%data @test         0x0 /8
%data @unlink_args  0x0 /4

_entry:
    write32     &unlink_args  0       &path
    syscall 10  &unlink_args  &ok
    ne          test        ok     0
    jtrue       &on_error   test

    ret

on_error:
    write64     &ok 0          0x1122
    ret
