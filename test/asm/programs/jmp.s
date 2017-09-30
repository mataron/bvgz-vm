
%data @num 0x0 /8
%data @test 0x0 /8
%data @postest 0x0 /8

_entry:
    cp64 num   2

decrement:
    sub num 1
    jmp &loop1

increment:
    add num 1
    jmp &loop2

loop1:
    ge test num 0
    le postest num 0xff ; ensure it didn't overflow
    l_and test postest
    jtrue &decrement test

    cp64 num 0
loop2:
    gt test num 2
    jfalse &increment test
