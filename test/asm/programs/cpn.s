
; all vars are 8 bytes long (64 bits)
%data @v_s8  0x00
%data @v_s16 0x0000
%data @v_s32 0x00000000
%data @v_s64 0x0 /8
%data @m_s8  0x00
%data @m_s16 0x0000
%data @m_s32 0x00000000
%data @m_s64 0x0 /8


_entry:
    write8  &v_s8 0  0x11
    write16 &v_s16 0 0x1122
    write32 &v_s32 0 0x11223344
    write64 &v_s64 0 0x1122334455667788
    write8  &m_s8 0 v_s8
    write16 &m_s16 0 v_s16
    write32 &m_s32 0 v_s32
    write64 &m_s64 0 v_s64
