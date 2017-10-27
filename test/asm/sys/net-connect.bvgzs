
%data @ok           0x1122334455667788 /8
%data @net_ip		0x11223344
%data @net_port		0x1122
%data @fd           0x0 /8
%data @err          0x0 /8
%data @test         0x0 /8
%data @ptr			0x0 /8
%data @arg			0x0 /8
%data @scall_args	0x0 /64
%data @c_cb_args	0x0 /16

_entry:
	; socket()
    write64     &scall_args 0  1
    syscall 15  &scall_args &fd
    eq          test        fd 0
    jtrue       &on_error   test

	; connect()
	write64		&scall_args	0	fd
	write64		&scall_args	8	&net_ip
	write64		&scall_args 12	&c_cb_args
	write64		&scall_args 16	&on_connect
	syscall 17	&scall_args	&err
    ne          test        err     0
    jtrue       &on_error   test

	ret ; exit main process

on_connect:
	argv		ptr
	; test errno
	read64      arg     ptr     8
    eq          test    arg     0
    jfalse      &on_error       test

	jmp	&socket_close
	; ret

socket_close:
	; close()
    syscall 5   &fd         &ok
    ret

on_error:
    write64     &ok			0	0x1122
    ret
