
%data @ok           0x0 /8
%data @n_saved      0x0 /8
%data @n_total      0x0 /8
%data @n_buf        0x0 /8
%data @path         "test.dir.456"
%data @test         0x0 /8
%data @rdir_args    0x0 /28
%data @buf          0x0 /512

_entry:
    write32     &rdir_args  0       &path
    write32     &rdir_args  4       &buf
    write64     &rdir_args  8       512
    write32     &rdir_args  16      &n_saved
    write32     &rdir_args  20      &n_total
    write32     &rdir_args  24      &n_buf
    syscall 14  &rdir_args  &ok
    ne          test        ok     0
    jtrue       &on_error   test

    ret

on_error:
    cp64        ok          0x1122
    ret
