
; all vars are 8 bytes long (64 bits)
%data @v_s8  0x00
%data @v_s16 0x0000
%data @v_s32 0x00000000
%data @v_s64 0x0000000000000000
%data @m_s8  0x00
%data @m_s16 0x0000
%data @m_s32 0x00000000
%data @m_s64 0x0000000000000000


_entry:
    set8  v_s8  0x11
    set16 v_s16 0x1122
    set32 v_s32 0x11223344
    set64 v_s64 0x1122334455667788
    set8  m_s8 v_s8
    set16 m_s16 v_s16
    set32 m_s32 v_s32
    set64 m_s64 v_s64
