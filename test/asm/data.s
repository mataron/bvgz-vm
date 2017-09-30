%data =instn-only.s ; 84 bytes

%data @foo =instn-only.s ; 84 bytes

%data @bar "\nstn-only.s" ; 11+1 bytes

; ONLY one warning here (trim)
%data @bar2 "sdf\nstn\"-only.s" - 4 ; 15+1 bytes -> 4

%data "sdf\nstn\"-only.s" - 53 ; 15+1 bytes -> 53

; 2 x 1 byte:
%data 0x1
%data 0x11
; 2 x 2 bytes:
%data 0x112
%data 0x1122
; 2 x 4 bytes:
%data @ptr 0x1122334
%data @ptr2 0x11223344
; 2 x 8 bytes:
%data 0x112233445566778
%data 0x1122334455667788

%data 0x0 - 8

; test data labels ::
add bar, foo, 0x12184
deref64 ptr &ptr
