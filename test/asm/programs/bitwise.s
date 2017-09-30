
; all vars are 8 bytes long (64 bits)
%data @v_and 0x0000000000000000
%data @v_or  0x0000000000000000
%data @v_not 0x0000000000000000
%data @v_xor 0x0000000000000000
%data @v_shl 0x0000000000000000
%data @v_shr 0x0000000000000000


_entry:
    and v_and 0x44 0x4
    or v_or 0x40 0x4
    not v_not 0xf0f0f0f0f0f0f0f0
    xor v_xor 0x44 0x4
    shl v_shl 0x400 8
    shr v_shr 0x400 8
