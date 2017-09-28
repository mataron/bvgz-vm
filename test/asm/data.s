%data =instn-only.s ; 84 bytes

%data @foo =instn-only.s ; 84 bytes

%data @bar "\nstn-only.s" ; 11+1 bytes

; ONLY one warning here (trim)
%data @bar2 "sdf\nstn\"-only.s" - 4 ; 15+1 bytes

; 2 x 1 byte:
%data 0x1
%data 0x11
; 2 x 2 bytes:
%data 0x112
%data 0x1122
; 2 x 4 bytes:
%data 0x1122334
%data 0x11223344
; 2 x 8 bytes:
%data 0x112233445566778
%data 0x1122334455667788
