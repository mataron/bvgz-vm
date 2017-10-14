
%data @ok           0x1122334455667788 /8
%data @net_ip		0x11223344
%data @net_port		0x1122
%data @fd           0x0 /8
%data @size_read	0x0 /8
%data @err          0x0 /8
%data @test         0x0 /8
%data @ptr			0x0 /8
%data @arg			0x0 /8
%data @scall_args	0x0 /64
%data @cb_args		0x0 /24
%data @rqst			"GET / HTTP/1.1\n\n"
%data @resp			0x0 /10240

_entry:
	; socket()
    cp64        scall_args  1
    syscall 15  &scall_args &fd
    eq          test        fd 0
    jtrue       &on_error   test

	; connect()
	write64		&scall_args	0	fd
	write64		&scall_args	8	&net_ip
	write64		&scall_args 12	&cb_args
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

	write64		&scall_args	0	fd
	write64		&scall_args 8	&rqst
	write64		&scall_args 16	17
	write64		&scall_args 24	&cb_args
	write64		&scall_args 32	&on_rqst_sent
	syscall 7	&scall_args	&err
    ne          test        err     0
    jtrue       &on_error   test
	ret

on_rqst_sent:
	argv		ptr
	; test errno
	read64      arg     ptr     8
    eq          test    arg     0
    jfalse      &on_error       test
	; test size transmitted
	read64      arg     ptr     16
    eq          test    arg     17
    jfalse      &on_error       test

read_reply:
	write64     &scall_args 0   fd
    write64     &scall_args 8   &resp
    write64     &scall_args 16  10240
    write64     &scall_args 24  &cb_args
    write64     &scall_args 32  &on_data_read
    syscall 6   &scall_args &err

    ne          test        err     0
    jtrue       &on_error   test

	ret

on_data_read:
	argv		ptr
	; test errno
	read64      arg     ptr     8
    eq          test    arg     0
    jfalse      &on_error       test
	; test size transmitted
	read64      arg     ptr     16
    gt          test    arg     16 ; some number...
	cp64		size_read		arg

	; fall through to socket_close!

socket_close:
	; close()
    syscall 5   &fd         &ok
    ret

on_error:
    cp64        ok          0x1122
    ret
