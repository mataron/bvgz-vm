
%data @ok           0x1122334455667788 /8
%data @pid			0x0 /8
%data @err          0x0 /8
%data @test         0x0 /8
%data @ptr			0x0 /8
%data @arg			0x0 /8
%data @scall_args	0x0 /24
%data @ls			"/bin/ls"
%data @ls_arg		"-lsha"
%data @argv			0x0 /8
%data @cb_args		0x0 /56

_entry:
	; setup argv
	write32		&argv 0			&ls
	write32		&argv 4			&ls_arg

	; exec()
	write64		&scall_args	0	2
	write32		&scall_args	8	&argv
	syscall 24	&scall_args	&pid
	eq			test		pid	0
	jtrue		&on_error	test

	; on-exit
	write64		&scall_args 0	pid
	write64		&scall_args 8	&cb_args
	write64		&scall_args 16	&on_ls_exit
	syscall 27	&scall_args &err
    ne          test        err     0
    jtrue       &on_error   test

	; kill it!
	write64		&scall_args 0	pid
	syscall 26	&scall_args &err
    ne          test        err     0
    jtrue       &on_error   test

	; exit main procedure
	ret

on_ls_exit:
	argv		ptr
	; arg 0: pid
	read64      arg     	ptr     0
	eq			test		arg		pid
	jfalse		&on_error	test
	; arg 1: exited?
	read64      arg     	ptr     8
	eq			test		arg		0
	jfalse		&on_error	test
	; arg 3: signaled?
	read64      arg     	ptr     24
	eq			test		arg		1
	jfalse		&on_error	test
	; arg 4: signal == TERM
	read64      arg     	ptr     32
	eq			test		arg		15
	jfalse		&on_error	test

	; done!
    cp64        ok         1
	ret

on_error:
    cp64        ok          0x1122
    ret
