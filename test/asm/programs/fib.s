
%data @num 0x0 /8

;; -- register area:
%data @r_ptr    0x0 /4 ; ptr to func local area (args)
%data @r_N      0x0 /8 ; value of current number
%data @r_Nm1    0x0 /8 ; value of current number - 1
%data @r_Nm2    0x0 /8 ; value of current number - 2
%data @r_term   0x0 /8 ; terminating condition check
%data @r_ret    0x0 /4 ; ptr to ret value of callee func

;; -- free area:
%data @mem 0x0 /512

_entry:
    ; cp64 mem 7 :: THIS IS SET BY THE HOST!!
    call &fib &mem &num
    ret

fib:
    argv        r_ptr
    read64      r_N     r_ptr
    le          r_term  r_N     1
    jtrue       &fib_done       r_term

    ; next func call will put result next to our arg
    cp32        r_ret   r_ptr
    add         r_ret   8
    ; skip our arg & next func's ret area to set the argument
    add         r_ptr   16
    sub         r_N     1
    write64     r_ptr   r_N
    ; ret & args set, call the thing!
    call        &fib    r_ptr  r_ret

    ; restore r_ptr:
    argv        r_ptr
    read64      r_N     r_ptr
    ; next func call will put result next to previous ret
    cp32        r_ret   r_ptr
    add         r_ret   16
    ; skip our arg & next func's ret area to set the argument
    add         r_ptr   24
    sub         r_N     2
    write64     r_ptr   r_N
    ; ret & args set, call the thing!
    call        &fib    r_ptr  r_ret

    ; restore r_ptr:
    argv        r_ptr
    ; copy the two numbers
    read64      r_Nm1   r_ptr   8
    read64      r_Nm2   r_ptr   16

    ; add the two & return them
    add         r_N     r_Nm1   r_Nm2

fib_done:
    retv r_N
    ret
