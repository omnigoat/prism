; Standard Windows/Controls Library
; Created by Bryant Keller

%ifndef __STDWIN_INC__
%define __STDWIN_INC__

; StdWindow szAppClass, szTitle, xpos, ypos, wwd, wht, menu, hinst
%macro StdWindow 8
     push dword NULL
     push dword %8
     push dword %7
     push dword NULL
     push dword %6
     push dword %5
     push dword %4
     push dword %3
     push dword WS_OVERLAPPEDWINDOW + WS_VISIBLE
     push dword %2
     push dword %1
     push dword WS_EX_OVERLAPPEDWINDOW
     invoke CreateWindowExA
%endmacro

; ButtonCtl szText, dwID, xpos, ypos, wwd, wht, prnt, hinst
%macro ButtonCtl 8
jmp %%n
%%cls: db "BUTTON", 0
%%n: push dword NULL
     push dword %8
     push dword %2
     push dword %7
     push dword %6
     push dword %5
     push dword %4
     push dword %3
     push dword WS_CHILD + WS_VISIBLE
     push dword %1
     push dword %%cls
     push dword NULL
     invoke CreateWindowExA
%endmacro

; EditCtl szText, dwID, xpos, ypos, wwd, wht, prnt, hinst
%macro EditCtl 8
jmp %%n
%%cls: db "EDIT", 0
%%n: push dword NULL
     push dword %8
     push dword %2
     push dword %7
     push dword %6
     push dword %5
     push dword %4
     push dword %3
     push dword WS_CHILD + WS_VISIBLE + ES_LEFT
     push dword %1
     push dword %%cls
     push dword NULL
     invoke CreateWindowExA
%endmacro

; StaticCtl szText, dwID, xpos, ypos, wwd, wht, prnt, hinst
%macro StaticCtl 8
jmp %%n
%%cls: db "STATIC", 0
%%n: push dword NULL
     push dword %8
     push dword %2
     push dword %7
     push dword %6
     push dword %5
     push dword %4
     push dword %3
     push dword WS_CHILD + WS_VISIBLE + SS_CENTER + SS_NOPREFIX
     push dword %1
     push dword %%cls
     push dword NULL
     invoke CreateWindowExA
%endmacro

; ComboCtl szText, dwID, xpos, ypos, wwd, wht, prnt, hinst
%macro ComboCtl 8
jmp %%n
%%cls: db "COMBOBOX", 0
%%n: push dword NULL
     push dword %8
     push dword %2
     push dword %7
     push dword %6
     push dword %5
     push dword %4
     push dword %3
     push dword WS_CHILD + WS_VISIBLE + CBS_DROPDOWN + CBS_SORT
     push dword %1
     push dword %%cls
     push dword NULL
     invoke CreateWindowExA
%endmacro

; ListBoxCtl szText, dwID, xpos, ypos, wwd, wht, prnt, hinst
%macro ListBoxCtl 8
jmp %%n
%%cls: db "LISTBOX", 0
%%n: push dword NULL
     push dword %8
     push dword %2
     push dword %7
     push dword %6
     push dword %5
     push dword %4
     push dword %3
     push dword WS_CHILD + WS_VISIBLE + LBS_HASSTRINGS + LBS_SORT
     push dword %1
     push dword %%cls
     push dword NULL
     invoke CreateWindowExA
%endmacro

%endif
