
%data @ok           0x0 /8
%data @path         "test.dir.123"
%data @test         0x0 /8
%data @rmdir_args   0x0 /4

_entry:
    write32     &rmdir_args  0       &path
    syscall 13  &rmdir_args  &ok
    ne          test        ok     0
    jtrue       &on_error   test

    ret

on_error:
    cp64        ok          0x1122
    ret
