
%data @ok           0x1122334455667788 /8
%data @net_ip		0x0 /4
%data @net_port		0x0 /2
%data @fd           0x0 /8
%data @clifd        0x0 /8
%data @err          0x0 /8
%data @test         0x0 /8
%data @ptr			0x0 /8
%data @arg			0x0 /8
%data @scall_args	0x0 /64
%data @cb_args		0x0 /24
%data @buf			0x0 /1024
%data @bufsz		0x0 /8

_entry:
	; socket()
    write64		&scall_args	0 1
    syscall 15  &scall_args &fd
    eq          test        fd 0
    jtrue       &on_error   test

	; bind()
	write64		&scall_args	0	fd
	write64		&scall_args	8	&net_ip
	syscall 16	&scall_args	&err
    ne          test        err     0
    jtrue       &on_error   test

	; listen()
	write64		&scall_args	0	fd
	syscall 18	&scall_args	&err
    ne          test        err     0
    jtrue       &on_error   test

	; accept()
	write64		&scall_args	0	fd
	write64		&scall_args	8	&cb_args
	write64		&scall_args	12	&on_accept
	syscall 19	&scall_args	&err
    ne          test        err     0
    jtrue       &on_error   test

	ret ; exit main process

on_accept:
	argv		ptr
	; test errno
	read64      arg     ptr     8
    eq          test    arg     0
    jfalse      &on_error       test

	read64      clifd     ptr	0

	; read()
	write64		&scall_args	0	clifd
	write64		&scall_args 8	&buf
	write64		&scall_args 16	1024
	write64		&scall_args 24	&cb_args
	write64		&scall_args 32	&on_data_read
	syscall 6	&scall_args	&err
    ne          test        err     0
    jtrue       &on_error   test
	ret

on_data_read:
	argv		ptr
	; test errno
	read64      arg     ptr     8
    eq          test    arg     0
    jfalse      &on_error       test

	read64      bufsz     ptr	0

	; write()
	write64		&scall_args	0	clifd
	write64		&scall_args 8	&buf
	write64		&scall_args 16	bufsz
	write64		&scall_args 24	&cb_args
	write64		&scall_args 32	&on_data_written
	syscall 7	&scall_args	&err
    ne          test        err     0
    jtrue       &on_error   test
	ret

on_data_written:
	argv		ptr
	; test errno
	read64      arg     ptr     8
    eq          test    arg     0
    jfalse      &on_error       test
	; test size
	read64      arg     ptr     16
    eq          test    arg     bufsz
    jfalse      &on_error       test

	; close(clifd)
	syscall 5   &clifd         &err
    ne          test        err     0
    jtrue       &on_error   test

	; fall through to socket_close!

socket_close:
	; close()
    syscall 5   &fd         &ok
    ret

on_error:
    write64     &ok 0       0x1122
    ret
