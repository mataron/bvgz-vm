
%data @ok           0x1122334455667788 /8
%data @pid			0x0 /8
%data @err          0x0 /8
%data @test         0x0 /8
%data @ptr			0x0 /8
%data @arg			0x0 /8
%data @scall_args	0x0 /32
%data @cb_args		0x0 /56
%data @codesz		0x0 /8
%data @memsz		0x0 /8
%data @entry		0x0 /8
%data @data			0x0 /1


_entry:
	; store into 'ptr' the address of the memory segment
	write64		&ptr 0			&data
	add			ptr				codesz

	; exec()
	write32		&scall_args	0	&data
	write64		&scall_args	4	codesz
	write32		&scall_args	12	ptr
	write64		&scall_args	16	memsz
	write64		&scall_args	24	entry
	syscall 25	&scall_args	&pid
	eq			test		pid	0
	jtrue		&on_error	test

	; on-exit
	write64		&scall_args 0	pid
	write64		&scall_args 8	&cb_args
	write64		&scall_args 16	&on_ls_exit
	syscall 27	&scall_args &err
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
