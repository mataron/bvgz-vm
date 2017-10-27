
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
%data @scall_args2	0x0 /8
%data				&cb_args /8
%data				&on_ls_exit /8

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
	write64		&scall_args2 0	pid
	syscall 27	&scall_args2 &err
    ne          test        err     0
    jtrue       &on_error   test

	; exit main procedure
	ret

on_ls_exit:
	argv		ptr
	read64      arg     	ptr     0
	eq			test		arg		pid
	jfalse		&on_error	test

	; done!
    write64     &ok 0         1
	ret

on_error:
    write64     &ok 0          0x1122
    ret
