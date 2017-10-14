
%data @dest 0x0 /32
%data 0x0 /32
%data @src "this is a nice string"
%data @srcend 0x0 /32
%data @num 0x0 /8

_entry:
    write32 &num 0 &srcend
    sub num &src
    cp dest, src, num
