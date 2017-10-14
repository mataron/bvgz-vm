
%data @b8   0x00
%data @b16  0x0000
%data @b32  0x00000000
%data @b64  0x0 /8
%data 0x0 /32  ; spacing
%data @data 0x1122334455667788
%data 0x0 /32  ; spacing
%data @ptr  0x00000000


_entry:
    write32 &ptr 0  &data
    read8   b8   ptr    0
    read16  b16  ptr    0
    read32  b32  ptr    0
    read64  b64  ptr    0
