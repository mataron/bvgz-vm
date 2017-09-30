
%data @num 0x0 /8
%data @test 0x0 /8
%data @postest 0x0 /8
%data @tmp 0x0 /8
%data @ptr 0x0 /4

_entry:
    set64 num   2

loop1:
    call &dec_f &num &num
    ge test num 0
    le postest num 0xff ; ensure it didn't overflow
    l_and test postest
    jtrue &loop1 test

past_loop1:
    set64 num 0

loop2:
    call &inc_f &num &num
    gt test num 2
    jfalse &loop2 test

past_loop2:
    ret

;; functions:
dec_f:
    argv ptr
    deref64 tmp ptr
    sub tmp 1
    retv tmp
    ret

inc_f:
    argv ptr
    deref64 tmp ptr
    add tmp 1
    retv tmp
    ret
