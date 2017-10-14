
%data @ok           0x1122334455667788 /8
%data @fd           0x0 /8
%data @size         0x0 /8
%data @done         0x0 /8
%data @err          0x1122334455667788 /8
%data @ptr          0x0 /8
%data @path         "Makefile"
%data @open_args    0x0 /12
%data @test         0x0 /8
%data @next_ptr     0x0 /8
%data @read_args    0x0 /40
%data @read_ret     0x0 /8
%data @read_cb      0x0 /24
%data @arg          0x0 /8
%data @seek_args    0x0 /24
%data @data         0x0 /40960

_entry:
    cp32        open_args   &path
    write64     &open_args  4  0x0

    syscall 8   &open_args  &fd
    eq          test        fd 0
    jtrue       &on_error   test

    write64     &seek_args  0   fd
    write64     &seek_args  8   -768
    write64     &seek_args  16  2 ; end of file
    syscall 11  &seek_args  &err

    ne          test        err     0
    jtrue       &on_error   test

    call        &file_read  0   read_ret
    eq          test        ok  0x1122
    jtrue       &on_error   test

wait_till_read_done:
    eq          test        done    0
    jfalse      &file_close test
    syscall 4   0 0
    jmp         &wait_till_read_done

file_close:
    syscall 5   &fd         &ok
    ret

file_read:
    cp64        next_ptr    &data
    add         next_ptr    size

    write64     &read_args  0       fd
    write64     &read_args  8       next_ptr
    write64     &read_args  16      512
    write64     &read_args  24      &read_cb
    write64     &read_args  32      &read_callback
    syscall 6   &read_args  &err

    ne          test        err     0
    jtrue       &on_error   test
    ret

read_callback:
    argv        ptr

    ; 1st arg: fd
    read64      arg     ptr     0
    eq          test    arg     fd
    jfalse      &on_error       test
    ; 2nd arg: errno
    read64      arg     ptr     8
    eq          test    arg     0
    jfalse      &on_error       test
    ; 3rd arg: size
    read64      arg     ptr     16
    eq          test    arg     0
    jtrue       &set_done       test
    add         size    arg

    call        &file_read  0   read_ret
    eq          test        ok  0x1122
    jtrue       &on_error   test
    ret
set_done:
    cp64        done    1
    ret

on_error:
    cp64        ok          0x1122
    ret
