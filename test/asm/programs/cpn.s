
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
    cp8  v_s8  0x11
    cp16 v_s16 0x1122
    cp32 v_s32 0x11223344
    cp64 v_s64 0x1122334455667788
    cp8  m_s8 v_s8
    cp16 m_s16 v_s16
    cp32 m_s32 v_s32
    cp64 m_s64 v_s64
