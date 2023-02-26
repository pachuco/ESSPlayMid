.686p
.mmx
.model flat

Voice       struc ; (sizeof=0x1A, mappedto_41) ; XREF: .data:_voice_table/r
flags1      db ?            ; XREF: voice_off(x)+54/w
                    ; find_voice(x,x,x,x)+7F/o ...
field_1     db ?
timer       dw ?            ; XREF: voice_off(x)+5B/w
                    ; find_voice(x,x,x,x)+BF/r ...
channel     db ?            ; XREF: find_voice(x,x,x,x)+93/r
                    ; find_voice(x,x,x,x)+10A/r ...
field_5     db ?            ; XREF: MidiAllNotesOff()+1/o
                    ; find_voice(x,x,x,x)+9F/r ...
flags2      db ?            ; XREF: setup_voice(x,x,x,x,x)+E0/w
field_7     db ?
field_8     dw 4 dup(?)     ; XREF: setup_operator(x,x,x,x,x,x,x,x,x)+223/w
                    ; MidiPitchBend(x,x)+6D/r
field_10    db ?            ; XREF: setup_voice(x,x,x,x,x)+D7/w
field_11    db 4 dup(?)     ; XREF: setup_operator(x,x,x,x,x,x,x,x,x)+1D8/w
field_15    db 4 dup(?)     ; XREF: setup_operator(x,x,x,x,x,x,x,x,x)+131/w
field_19    db ?
Voice       ends







; export
extrn _MidiPitchBend@8:proc
extrn _MidiCalcFAndB@8:proc
extrn _NATV_CalcVolume@12:proc
public _NATV_CalcNewVolume@4
public _NATV_CalcBend@12
extrn _voice_off@4:proc
extrn _note_off@8:proc
public _note_on@12
extrn _setup_voice@20:proc
public _find_voice@16
public _steal_voice@4
public _setup_operator@36
extrn _voice_on@4:proc
public _hold_controller@8
public _fmreset@0
public _MidiAllNotesOff@0
public _MidiMessage@4

; import
extrn _fnum:word
extrn _gbVelocityAtten:byte
extrn _td_adjust_setup_operator:dword
extrn _pmask_MidiPitchBend:byte

extrn _gBankMem:dword
extrn _v1:dword
extrn _v2:dword
extrn _gwTimer:dword

extrn _voice_table:Voice
extrn _gbVelLevel:byte
extrn _pan_mask:byte
extrn _gbChanAtten:byte
extrn _giBend:word
extrn _hold_table:byte
extrn _program_table:byte
extrn _gbChanVolume:byte
extrn _gbChanExpr:byte
extrn _gbChanBendRange:byte
extrn _note_offs:byte


extrn _fmwrite231@8:proc
extrn _fmwrite21@8:proc


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.data


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
.code


















; void __stdcall MidiMessage(DWORD dwData)
_MidiMessage@4  proc near       ; CODE XREF: modSynthMessage(x,x,x,x,x)+111p
                    ; modSynthMessage(x,x,x,x,x)+1A3p ...

a3      = byte ptr -8
a2      = byte ptr -4
dwData      = dword ptr  8

        push    ebp
        mov ebp, esp
        push    ecx
        push    ecx
        mov edx, [ebp+dwData]
        xor ecx, ecx
        mov eax, edx
        push    ebx
        mov bl, dl
        mov cl, dh
        shr eax, 10h
        and edx, 0F0h
        and bl, 0Fh
        and al, 7Fh
        and cl, 7Fh
        push    esi
        sub edx, 80h ; '€'
        push    edi
        mov byte ptr [ebp+dwData], bl
        mov [ebp+a3], al
        mov [ebp+a2], cl
        jz  loc_6BC0711B
        sub edx, 10h
        jz  loc_6BC07107
        sub edx, 20h ; ' '
        jz  short loc_6BC06F08
        sub edx, 10h
        jz  short loc_6BC06EFA
        sub edx, 20h ; ' '
        jnz loc_6BC07126
        movzx   ax, al
        movzx   cx, cl
        shl eax, 7
        or  eax, ecx
        push    eax     ; iBend
        push    [ebp+dwData]    ; bChannel
        call    _MidiPitchBend@8 ; MidiPitchBend(x,x)
        jmp loc_6BC07126
; ---------------------------------------------------------------------------

loc_6BC06EFA:               ; CODE XREF: MidiMessage(x)+4Aj
        movzx   eax, bl
        mov _program_table[eax], cl
        jmp loc_6BC07126
; ---------------------------------------------------------------------------

loc_6BC06F08:               ; CODE XREF: MidiMessage(x)+45j
        movzx   ecx, cl
        cmp ecx, 40h ; '@'
        jg  loc_6BC06FCD
        jz  loc_6BC06FBB
        sub ecx, 6
        jz  short loc_6BC06F9B
        dec ecx
        jz  short loc_6BC06F6F
        dec ecx
        jz  short loc_6BC06F3D
        dec ecx
        dec ecx
        jz  short loc_6BC06F3D
        dec ecx
        jnz loc_6BC07126
        movzx   ecx, bl
        and al, 7Fh
        mov _gbChanExpr[ecx], al
        jmp short loc_6BC06F8E
; ---------------------------------------------------------------------------

loc_6BC06F3D:               ; CODE XREF: MidiMessage(x)+99j
                    ; MidiMessage(x)+9Dj
        cmp al, 50h ; 'P'
        jbe short loc_6BC06F50
        movzx   eax, bl
        mov _pan_mask[eax], 20h ; ' '
        jmp loc_6BC07126
; ---------------------------------------------------------------------------

loc_6BC06F50:               ; CODE XREF: MidiMessage(x)+B5j
        cmp al, 30h ; '0'
        movzx   eax, bl
        jnb short loc_6BC06F63
        mov _pan_mask[eax], 10h
        jmp loc_6BC07126
; ---------------------------------------------------------------------------

loc_6BC06F63:               ; CODE XREF: MidiMessage(x)+CBj
        mov _pan_mask[eax], 30h ; '0'
        jmp loc_6BC07126
; ---------------------------------------------------------------------------

loc_6BC06F6F:               ; CODE XREF: MidiMessage(x)+96j
        mov dl, al
        shr edx, 2
        movzx   ecx, bl
        and edx, 1Fh
        and al, 7Fh
        mov _gbChanVolume[ecx], al
        mov dl, _gbVelocityAtten[edx]
        mov _gbChanAtten[ecx], dl

loc_6BC06F8E:               ; CODE XREF: MidiMessage(x)+B1j
        push    [ebp+dwData]    ; bChannel
        call    _NATV_CalcNewVolume@4 ; NATV_CalcNewVolume(x)
        jmp loc_6BC07126
; ---------------------------------------------------------------------------

loc_6BC06F9B:               ; CODE XREF: MidiMessage(x)+93j
        movzx   ecx, bl
        mov dl, _hold_table[ecx]
        and dl, 6
        cmp dl, 6
        jnz loc_6BC07126
        mov byte ptr _gbChanBendRange[ecx], al
        jmp loc_6BC07126
; ---------------------------------------------------------------------------

loc_6BC06FBB:               ; CODE XREF: MidiMessage(x)+8Aj
        movzx   eax, al
        push    eax     ; a3
        movzx   eax, bl
        push    eax     ; a2
        call    _hold_controller@8 ; hold_controller(x,x)
        jmp loc_6BC07126
; ---------------------------------------------------------------------------

loc_6BC06FCD:               ; CODE XREF: MidiMessage(x)+84j
        cmp ecx, 78h ; 'x'
        jg  short loc_6BC0702A
        jz  short loc_6BC07049
        sub ecx, 62h ; 'b'
        jz  short loc_6BC0701B
        dec ecx
        jz  short loc_6BC0700C
        dec ecx
        jz  short loc_6BC06FF9
        dec ecx
        jnz loc_6BC07126
        test    al, al
        movzx   eax, bl
        jnz short loc_6BC0700F
        or  _hold_table[eax], 4
        jmp loc_6BC07126
; ---------------------------------------------------------------------------

loc_6BC06FF9:               ; CODE XREF: MidiMessage(x)+153j
        test    al, al
        movzx   eax, bl
        jnz short loc_6BC0701E
        or  _hold_table[eax], 2
        jmp loc_6BC07126
; ---------------------------------------------------------------------------

loc_6BC0700C:               ; CODE XREF: MidiMessage(x)+150j
        movzx   eax, bl

loc_6BC0700F:               ; CODE XREF: MidiMessage(x)+161j
        and _hold_table[eax], 0FBh
        jmp loc_6BC07126
; ---------------------------------------------------------------------------

loc_6BC0701B:               ; CODE XREF: MidiMessage(x)+14Dj
        movzx   eax, bl

loc_6BC0701E:               ; CODE XREF: MidiMessage(x)+174j
        and _hold_table[eax], 0FDh
        jmp loc_6BC07126
; ---------------------------------------------------------------------------

loc_6BC0702A:               ; CODE XREF: MidiMessage(x)+146j
        cmp ecx, 79h ; 'y'
        jz  short loc_6BC0709E
        cmp ecx, 7Bh ; '{'
        jz  short loc_6BC07071
        jle loc_6BC07126
        cmp ecx, 7Dh ; '}'
        jle short loc_6BC07049
        cmp ecx, 7Fh ; ''
        jle short loc_6BC07071
        jmp loc_6BC07126
; ---------------------------------------------------------------------------

loc_6BC07049:               ; CODE XREF: MidiMessage(x)+148j
                    ; MidiMessage(x)+1B3j
        xor edi, edi
        mov esi, offset _voice_table.channel

loc_6BC07050:               ; CODE XREF: MidiMessage(x)+1E0j
        test    byte ptr [esi-4], 1
        jz  short loc_6BC07060
        cmp [esi], bl
        jnz short loc_6BC07060
        push    edi     ; a2
        call    _voice_off@4    ; voice_off(x)

loc_6BC07060:               ; CODE XREF: MidiMessage(x)+1CAj
                    ; MidiMessage(x)+1CEj
        add esi, 1Ah
        inc edi
        cmp esi, (offset _voice_table.channel+1D4h)
        jl  short loc_6BC07050
        jmp loc_6BC07126
; ---------------------------------------------------------------------------

loc_6BC07071:               ; CODE XREF: MidiMessage(x)+1A8j
                    ; MidiMessage(x)+1B8j
        xor edi, edi
        mov esi, offset _voice_table

loc_6BC07078:               ; CODE XREF: MidiMessage(x)+20Dj
        mov al, [esi]
        test    al, 1
        jz  short loc_6BC0708D
        cmp [esi+4], bl
        jnz short loc_6BC0708D
        test    al, 4
        jnz short loc_6BC0708D
        push    edi     ; a2
        call    _voice_off@4    ; voice_off(x)

loc_6BC0708D:               ; CODE XREF: MidiMessage(x)+1F2j
                    ; MidiMessage(x)+1F7j ...
        add esi, 1Ah
        inc edi
        cmp esi, (offset _voice_table.flags1+1D4h)
        jl  short loc_6BC07078
        jmp loc_6BC07126
; ---------------------------------------------------------------------------

loc_6BC0709E:               ; CODE XREF: MidiMessage(x)+1A3j
        movzx   esi, bl
        test    _hold_table[esi], 1
        jz  short loc_6BC070D8
        and [ebp+dwData], 0
        mov edi, offset _voice_table

loc_6BC070B3:               ; CODE XREF: MidiMessage(x)+24Cj
        mov al, [edi]
        test    al, 1
        jz  short loc_6BC070CA
        cmp [edi+4], bl
        jnz short loc_6BC070CA
        test    al, 4
        jz  short loc_6BC070CA
        push    [ebp+dwData]    ; a2
        call    _voice_off@4    ; voice_off(x)

loc_6BC070CA:               ; CODE XREF: MidiMessage(x)+22Dj
                    ; MidiMessage(x)+232j ...
        inc [ebp+dwData]
        add edi, 1Ah
        cmp edi, (offset _voice_table.flags1+1D4h)
        jl  short loc_6BC070B3

loc_6BC070D8:               ; CODE XREF: MidiMessage(x)+21Ej
        and _hold_table[esi], 0FEh
        mov _gbChanVolume[esi], 64h ; 'd'
        mov _gbChanExpr[esi], 7Fh ; ''
        mov _giBend[esi*2], 2000h
        mov _pan_mask[esi], 30h ; '0'
        mov byte ptr _gbChanBendRange[esi], 2
        jmp short loc_6BC07126
; ---------------------------------------------------------------------------

loc_6BC07107:               ; CODE XREF: MidiMessage(x)+3Cj
        test    al, al
        jz  short loc_6BC0711B
        push    dword ptr [ebp+a3] ; a3
        push    dword ptr [ebp+a2] ; a2
        push    [ebp+dwData]    ; channel
        call    _note_on@12 ; note_on(x,x,x)
        jmp short loc_6BC07126
; ---------------------------------------------------------------------------

loc_6BC0711B:               ; CODE XREF: MidiMessage(x)+33j
                    ; MidiMessage(x)+27Fj
        push    dword ptr [ebp+a2] ; a2
        push    [ebp+dwData]    ; bChannel
        call    _note_off@8 ; note_off(x,x)

loc_6BC07126:               ; CODE XREF: MidiMessage(x)+4Fj
                    ; MidiMessage(x)+6Bj ...
        pop edi
        pop esi
        pop ebx
        leave
        retn    4
_MidiMessage@4  endp





; void __stdcall MidiPitchBend(BYTE bChannel, int iBend)
ZZZ_MidiPitchBend@8 proc near      ; CODE XREF: MidiMessage(x)+66p

var_1C      = dword ptr -1Ch
i       = dword ptr -18h
j       = dword ptr -14h
var_10      = dword ptr -10h
var_C       = dword ptr -0Ch
var_8       = dword ptr -8
var_4       = dword ptr -4
bChannel    = byte ptr  8
iBend       = dword ptr  0Ch

        push    ebp
        mov ebp, esp
        sub esp, 1Ch
        mov ax, word ptr [ebp+iBend]
        and [ebp+var_10], 0
        and [ebp+var_8], 0
        push    ebx
        movzx   ebx, [ebp+bChannel]
        push    esi
        mov esi, offset _voice_table
        push    edi
        mov _giBend[ebx*2], ax
        mov [ebp+var_C], esi
        mov [ebp+i], 12h

loc_6BC06C84:               ; CODE XREF: MidiPitchBend(x,x)+ECj
        mov al, [ebp+bChannel]
        cmp [esi+4], al
        jnz loc_6BC06D2F
        test    byte ptr [esi], 1
        jz  loc_6BC06D2F
        xor edi, edi
        mov [ebp+j], 4
        mov [ebp+var_4], edi

loc_6BC06CA5:               ; CODE XREF: MidiPitchBend(x,x)+D5j
        mov al, _pmask_MidiPitchBend[edi]
        test    [esi+10h], al
        jnz short loc_6BC06D21
        movzx   ax, byte ptr _gbChanBendRange[ebx]
        push    eax     ; a3
        mov eax, [ebp+var_8]
        push    [ebp+iBend] ; iBend
        add eax, edi
        mov ax, _voice_table.field_8[eax*2]
        push    eax     ; a1
        call    _NATV_CalcBend@12 ; NATV_CalcBend(x,x,x)
        mov cl, [esi+edi+11h]
        shr cl, 2
        movzx   eax, ax
        and cl, 7
        push    ecx
        push    eax
        call    _MidiCalcFAndB@8 ; MidiCalcFAndB(x,x)
        movzx   ecx, word ptr [ebp+var_4]
        mov edx, [ebp+var_10]
        mov [ebp+var_1C], eax
        lea esi, [ecx+edx]
        xor ecx, ecx
        mov cl, ah
        movzx   eax, cl
        mov ecx, [ebp+var_C]
        mov cl, [ecx+edi+11h]
        and ecx, 0E0h   ; bytesWritten
        or  eax, ecx
        push    eax     ; a2
        lea eax, [esi+5]
        push    eax     ; a1
        call    _fmwrite231@8   ; fmwrite(x,x)
        movzx   eax, byte ptr [ebp+var_1C]
        add esi, 4
        push    eax     ; a2
        push    esi     ; a1
        call    _fmwrite231@8   ; fmwrite(x,x)
        mov esi, [ebp+var_C]

loc_6BC06D21:               ; CODE XREF: MidiPitchBend(x,x)+5Aj
        add [ebp+var_4], 8
        inc edi
        dec [ebp+j]
        jnz loc_6BC06CA5

loc_6BC06D2F:               ; CODE XREF: MidiPitchBend(x,x)+36j
                    ; MidiPitchBend(x,x)+3Fj
        add [ebp+var_8], 0Dh
        add [ebp+var_10], 20h ; ' '
        add esi, 1Ah
        dec [ebp+i]
        mov [ebp+var_C], esi
        jnz loc_6BC06C84
        pop edi
        pop esi
        pop ebx
        leave
        retn    8
ZZZ_MidiPitchBend@8 endp






; __stdcall MidiCalcFAndB(x, x)
ZZZ_MidiCalcFAndB@8 proc near      ; CODE XREF: setup_operator(x,x,x,x,x,x,x,x,x)+200p
                    ; MidiPitchBend(x,x)+8Ap

arg_0       = dword ptr  8
arg_4       = byte ptr  0Ch

        push    ebp
        mov ebp, esp
        mov ecx, [ebp+arg_0]
        mov eax, 400h

loc_6BC06621:               ; CODE XREF: MidiCalcFAndB(x,x)+14j
        cmp ecx, eax
        jb  short loc_6BC0662C
        shr ecx, 1
        inc [ebp+arg_4]
        jmp short loc_6BC06621
; ---------------------------------------------------------------------------

loc_6BC0662C:               ; CODE XREF: MidiCalcFAndB(x,x)+Dj
        cmp [ebp+arg_4], 7
        jbe short loc_6BC06636
        mov [ebp+arg_4], 7

loc_6BC06636:               ; CODE XREF: MidiCalcFAndB(x,x)+1Aj
        movzx   eax, [ebp+arg_4]
        shl eax, 0Ah
        or  eax, ecx
        pop ebp
        retn    8
ZZZ_MidiCalcFAndB@8 endp






; BYTE __stdcall NATV_CalcVolume(BYTE a1, BYTE a2, BYTE a3)
ZZZ_NATV_CalcVolume@12 proc near       ; CODE XREF: setup_operator(x,x,x,x,x,x,x,x,x)+137p
                    ; NATV_CalcNewVolume(x)+43p

a1      = byte ptr  8
arg_4       = dword ptr  0Ch
a3      = byte ptr  10h

        push    ebp
        mov ebp, esp
        movzx   eax, [ebp+a3]
        mov dl, _gbChanVolume[eax]
        test    dl, dl
        jnz short loc_6BC06D66
        mov al, 3Fh ; '?'
        jmp loc_6BC06E14
; ---------------------------------------------------------------------------

loc_6BC06D66:               ; CODE XREF: NATV_CalcVolume(x,x,x)+Fj
        mov ecx, [ebp+arg_4]
        sub ecx, 0
        jz  loc_6BC06DF5
        dec ecx
        jz  short loc_6BC06DD7
        dec ecx
        jz  short loc_6BC06DBB
        dec ecx
        jnz short loc_6BC06DF9
        mov al, _gbChanExpr[eax]
        cmp al, 40h ; '@'
        movzx   ecx, al
        jb  short loc_6BC06D92
        push    7Fh ; ''
        pop eax
        sub eax, ecx
        sar eax, 2
        jmp short loc_6BC06D9C
; ---------------------------------------------------------------------------

loc_6BC06D92:               ; CODE XREF: NATV_CalcVolume(x,x,x)+38j
        push    3Fh ; '?'
        pop eax
        sub eax, ecx
        sar eax, 1
        add eax, 10h

loc_6BC06D9C:               ; CODE XREF: NATV_CalcVolume(x,x,x)+42j
        cmp dl, 40h ; '@'
        movzx   ecx, dl
        jb  short loc_6BC06DAE
        push    7Fh ; ''
        pop edx
        sub edx, ecx
        sar edx, 2
        jmp short loc_6BC06DF1
; ---------------------------------------------------------------------------

loc_6BC06DAE:               ; CODE XREF: NATV_CalcVolume(x,x,x)+54j
        push    3Fh ; '?'
        pop edx
        sub edx, ecx
        sar edx, 1
        lea eax, [eax+edx+10h]
        jmp short loc_6BC06DFC
; ---------------------------------------------------------------------------

loc_6BC06DBB:               ; CODE XREF: NATV_CalcVolume(x,x,x)+28j
        movzx   ecx, _gbChanExpr[eax]
        push    7Fh ; ''
        pop eax
        sub eax, ecx
        push    7Fh ; ''
        movzx   ecx, dl
        pop edx
        sub edx, ecx
        sar eax, 3
        sar edx, 3
        jmp short loc_6BC06DF1
; ---------------------------------------------------------------------------

loc_6BC06DD7:               ; CODE XREF: NATV_CalcVolume(x,x,x)+25j
        movzx   ecx, _gbChanExpr[eax]
        push    7Fh ; ''
        pop eax
        sub eax, ecx
        push    7Fh ; ''
        movzx   ecx, dl
        pop edx
        sub edx, ecx
        sar eax, 4
        sar edx, 4

loc_6BC06DF1:               ; CODE XREF: NATV_CalcVolume(x,x,x)+5Ej
                    ; NATV_CalcVolume(x,x,x)+87j
        add eax, edx
        jmp short loc_6BC06DFC
; ---------------------------------------------------------------------------

loc_6BC06DF5:               ; CODE XREF: NATV_CalcVolume(x,x,x)+1Ej
        xor eax, eax
        jmp short loc_6BC06DFC
; ---------------------------------------------------------------------------

loc_6BC06DF9:               ; CODE XREF: NATV_CalcVolume(x,x,x)+2Bj
        mov eax, dword ptr [ebp+a1]

loc_6BC06DFC:               ; CODE XREF: NATV_CalcVolume(x,x,x)+6Bj
                    ; NATV_CalcVolume(x,x,x)+A5j ...
        mov cl, [ebp+a1]
        and cl, 3Fh
        add cl, al
        cmp cl, 3Fh ; '?'
        jbe short loc_6BC06E0B
        mov cl, 3Fh ; '?'

loc_6BC06E0B:               ; CODE XREF: NATV_CalcVolume(x,x,x)+B9j
        mov al, [ebp+a1]
        and ax, 0C0h
        or  eax, ecx

loc_6BC06E14:               ; CODE XREF: NATV_CalcVolume(x,x,x)+13j
        pop ebp
        retn    0Ch
ZZZ_NATV_CalcVolume@12 endp





; void __stdcall NATV_CalcNewVolume(BYTE bChannel)
_NATV_CalcNewVolume@4 proc near     ; CODE XREF: MidiMessage(x)+107p

a3      = byte ptr -8
a1      = word ptr -4
bChannel    = byte ptr  8

        push    ebp
        mov ebp, esp
        push    ecx
        push    ecx
        push    ebx
        push    esi
        push    edi
        mov dword ptr [ebp+a1], 1
        mov esi, offset _voice_table.channel

loc_6BC06E2C:               ; CODE XREF: NATV_CalcNewVolume(x)+69j
        test    byte ptr [esi-4], 1
        jz  short loc_6BC06E73
        mov al, [esi]
        cmp al, [ebp+bChannel]
        jz  short loc_6BC06E3F
        cmp [ebp+bChannel], 0FFh
        jnz short loc_6BC06E73

loc_6BC06E3F:               ; CODE XREF: NATV_CalcNewVolume(x)+1Fj
        mov ebx, dword ptr [ebp+a1]
        mov [ebp+a3], al
        xor edi, edi

loc_6BC06E47:               ; CODE XREF: NATV_CalcNewVolume(x)+59j
        movzx   eax, byte ptr [esi+2]
        push    dword ptr [ebp+a3] ; a3
        xor ecx, ecx
        shr eax, cl
        and eax, 3
        push    eax     ; a2
        mov al, [edi+esi+11h]
        push    eax     ; a1
        call    _NATV_CalcVolume@12 ; NATV_CalcVolume(x,x,x)
        movzx   eax, al
        push    eax     ; a2
        push    ebx     ; a1
        call    _fmwrite231@8   ; fmwrite(x,x)
        inc edi
        add ebx, 8
        cmp edi, 4
        jl  short loc_6BC06E47

loc_6BC06E73:               ; CODE XREF: NATV_CalcNewVolume(x)+18j
                    ; NATV_CalcNewVolume(x)+25j
        add dword ptr [ebp+a1], 20h ; ' '
        add esi, 1Ah
        cmp dword ptr [ebp+a1], 241h
        jl  short loc_6BC06E2C
        pop edi
        pop esi
        pop ebx
        leave
        retn    4
_NATV_CalcNewVolume@4 endp







; __int16 __stdcall NATV_CalcBend(USHORT a1, USHORT iBend, USHORT a3)
_NATV_CalcBend@12 proc near     ; CODE XREF: setup_operator(x,x,x,x,x,x,x,x,x)+1F4p
                    ; MidiPitchBend(x,x)+76p

NATV_table1 = dword ptr -1C4h
NATV_table2 = dword ptr -0C4h
a1      = word ptr  8
iBend       = word ptr  0Ch
a3      = word ptr  10h

        push    ebp
        mov ebp, esp
        sub esp, 1C4h
        mov eax, 400h
        mov edx, 200h
        mov [ebp+NATV_table2+60h], eax
        mov [ebp+NATV_table1], eax
        add eax, 6
        mov [ebp+NATV_table2], 100h
        mov [ebp+NATV_table1+18h], eax
        mov [ebp+NATV_table1+1Ch], eax
        mov eax, 415h
        mov [ebp+NATV_table2+4], 10Fh
        mov [ebp+NATV_table2+8], 11Fh
        mov [ebp+NATV_table2+0Ch], 130h
        mov [ebp+NATV_table2+10h], 143h
        mov [ebp+NATV_table2+14h], 156h
        mov [ebp+NATV_table2+18h], 16Ah
        mov [ebp+NATV_table2+1Ch], 180h
        mov [ebp+NATV_table2+20h], 196h
        mov [ebp+NATV_table2+24h], 1AFh
        mov [ebp+NATV_table2+28h], 1C8h
        mov [ebp+NATV_table2+2Ch], 1E3h
        mov [ebp+NATV_table2+30h], edx
        mov [ebp+NATV_table2+34h], 21Eh
        mov [ebp+NATV_table2+38h], 23Fh
        mov [ebp+NATV_table2+3Ch], 261h
        mov [ebp+NATV_table2+40h], 285h
        mov [ebp+NATV_table2+44h], 2ABh
        mov [ebp+NATV_table2+48h], 2D4h
        mov [ebp+NATV_table2+4Ch], 2FFh
        mov [ebp+NATV_table2+50h], 32Dh
        mov [ebp+NATV_table2+54h], 35Dh
        mov [ebp+NATV_table2+58h], 390h
        mov [ebp+NATV_table2+5Ch], 3C7h
        mov [ebp+NATV_table2+64h], 43Dh
        mov [ebp+NATV_table2+68h], 47Dh
        mov [ebp+NATV_table2+6Ch], 4C2h
        mov [ebp+NATV_table2+70h], 50Ah
        mov [ebp+NATV_table2+74h], 557h
        mov [ebp+NATV_table2+78h], 5A8h
        mov [ebp+NATV_table2+7Ch], 5FEh
        mov [ebp+NATV_table2+80h], 659h
        mov [ebp+NATV_table2+84h], 6BAh
        mov [ebp+NATV_table2+88h], 721h
        mov [ebp+NATV_table2+8Ch], 78Dh
        mov [ebp+NATV_table2+90h], 800h
        mov [ebp+NATV_table2+94h], 87Ah
        mov [ebp+NATV_table2+98h], 8FBh
        mov [ebp+NATV_table2+9Ch], 983h
        mov [ebp+NATV_table2+0A0h], 0A14h
        mov [ebp+NATV_table2+0A4h], 0AAEh
        mov [ebp+NATV_table2+0A8h], 0B50h
        mov [ebp+NATV_table2+0ACh], 0BFDh
        mov [ebp+NATV_table2+0B0h], 0CB3h
        mov [ebp+NATV_table2+0B4h], 0D74h
        mov [ebp+NATV_table2+0B8h], 0E41h
        mov [ebp+NATV_table2+0BCh], 0F1Ah
        mov [ebp+NATV_table2+0C0h], 1000h
        mov [ebp+NATV_table1+4], 401h
        mov [ebp+NATV_table1+8], 402h
        mov [ebp+NATV_table1+0Ch], 403h
        mov [ebp+NATV_table1+10h], 404h
        mov [ebp+NATV_table1+14h], 405h
        mov [ebp+NATV_table1+20h], 407h
        mov [ebp+NATV_table1+24h], 408h
        mov [ebp+NATV_table1+28h], 409h
        mov [ebp+NATV_table1+2Ch], 40Ah
        mov [ebp+NATV_table1+30h], 40Bh
        mov [ebp+NATV_table1+34h], 40Ch
        mov [ebp+NATV_table1+38h], 40Dh
        mov [ebp+NATV_table1+3Ch], 40Eh
        mov [ebp+NATV_table1+40h], 40Fh
        mov [ebp+NATV_table1+44h], 410h
        mov [ebp+NATV_table1+48h], 411h
        mov [ebp+NATV_table1+4Ch], 412h
        mov [ebp+NATV_table1+50h], 413h
        mov [ebp+NATV_table1+54h], 414h
        mov [ebp+NATV_table1+58h], eax
        mov [ebp+NATV_table1+5Ch], eax
        mov [ebp+NATV_table1+60h], 416h
        mov [ebp+NATV_table1+64h], 417h
        mov [ebp+NATV_table1+68h], 418h
        mov [ebp+NATV_table1+6Ch], 419h
        add eax, 14h
        mov ecx, 2000h
        mov [ebp+NATV_table1+0ACh], eax
        mov [ebp+NATV_table1+0B0h], eax
        mov eax, dword ptr [ebp+iBend]
        mov [ebp+NATV_table1+70h], 41Ah
        cmp ax, cx
        mov [ebp+NATV_table1+74h], 41Bh
        mov [ebp+NATV_table1+78h], 41Ch
        mov [ebp+NATV_table1+7Ch], 41Dh
        mov [ebp+NATV_table1+80h], 41Eh
        mov [ebp+NATV_table1+84h], 41Fh
        mov [ebp+NATV_table1+88h], 420h
        mov [ebp+NATV_table1+8Ch], 421h
        mov [ebp+NATV_table1+90h], 422h
        mov [ebp+NATV_table1+94h], 423h
        mov [ebp+NATV_table1+98h], 424h
        mov [ebp+NATV_table1+9Ch], 425h
        mov [ebp+NATV_table1+0A0h], 426h
        mov [ebp+NATV_table1+0A4h], 427h
        mov [ebp+NATV_table1+0A8h], 428h
        mov [ebp+NATV_table1+0B4h], 42Ah
        mov [ebp+NATV_table1+0B8h], 42Bh
        mov [ebp+NATV_table1+0BCh], 42Ch
        mov [ebp+NATV_table1+0C0h], 42Dh
        mov [ebp+NATV_table1+0C4h], 42Eh
        mov [ebp+NATV_table1+0C8h], 42Fh
        mov [ebp+NATV_table1+0CCh], 430h
        mov [ebp+NATV_table1+0D0h], 431h
        mov [ebp+NATV_table1+0D4h], 432h
        mov [ebp+NATV_table1+0D8h], 433h
        mov [ebp+NATV_table1+0DCh], 434h
        mov [ebp+NATV_table1+0E0h], 435h
        mov [ebp+NATV_table1+0E4h], 436h
        mov [ebp+NATV_table1+0E8h], 437h
        mov [ebp+NATV_table1+0ECh], 438h
        mov [ebp+NATV_table1+0F0h], 439h
        mov [ebp+NATV_table1+0F4h], 43Ah
        mov [ebp+NATV_table1+0F8h], 43Bh
        mov [ebp+NATV_table1+0FCh], 43Ch
        jnz short loc_6BC065C7
        mov ax, [ebp+a1]
        jmp short locret_6BC06612
; ---------------------------------------------------------------------------

loc_6BC065C7:               ; CODE XREF: NATV_CalcBend(x,x,x)+40Fj
        cmp ax, 3F80h
        jb  short loc_6BC065D2
        mov eax, 4000h

loc_6BC065D2:               ; CODE XREF: NATV_CalcBend(x,x,x)+41Bj
        movzx   eax, ax
        sub eax, ecx
        movzx   ecx, [ebp+a3]
        imul    eax, ecx
        sar eax, 5
        add eax, 1800h
        mov ecx, eax
        sar ecx, 8
        sar eax, 2
        mov ecx, [ebp+ecx*4+NATV_table2]
        and eax, 3Fh
        imul    ecx, [ebp+eax*4+NATV_table1]
        sar ecx, 0Ah
        movzx   eax, cx
        movzx   ecx, [ebp+a1]
        imul    eax, ecx
        add eax, edx
        sar eax, 0Ah

locret_6BC06612:            ; CODE XREF: NATV_CalcBend(x,x,x)+415j
        leave
        retn    0Ch
_NATV_CalcBend@12 endp











; void __stdcall voice_off(signed int a2)
ZZZ_voice_off@4    proc near       ; CODE XREF: note_off(x,x)+35p
                    ; find_voice(x,x,x,x)+3Ap ...

a2      = dword ptr  4

        push    esi
        mov esi, [esp+4+a2]
        cmp esi, 10h
        push    0       ; a2
        jge short loc_6BC05EC3
        lea eax, [esi+240h]
        push    eax
        jmp short loc_6BC05EE9
; ---------------------------------------------------------------------------

loc_6BC05EC3:               ; CODE XREF: voice_off(x)+Aj
        jnz short loc_6BC05ED8
        push    250h        ; a1
        call    _fmwrite231@8   ; fmwrite(x,x)
        push    0
        push    251h
        jmp short loc_6BC05EE9
; ---------------------------------------------------------------------------

loc_6BC05ED8:               ; CODE XREF: voice_off(x):loc_6BC05EC3j
        push    252h        ; a1
        call    _fmwrite231@8   ; fmwrite(x,x)
        push    0       ; a2
        push    253h        ; a1

loc_6BC05EE9:               ; CODE XREF: voice_off(x)+13j
                    ; voice_off(x)+28j
        call    _fmwrite231@8   ; fmwrite(x,x)
        mov eax, esi
        mov cx, word ptr _gwTimer
        imul    eax, 1Ah
        inc word ptr _gwTimer
        pop esi
        mov byte ptr _voice_table.flags1[eax], 2
        mov _voice_table.timer[eax], cx
        retn    4
ZZZ_voice_off@4    endp




















































; void __stdcall note_off(unsigned __int8 bChannel, char a2)
ZZZ_note_off@8 proc near       ; CODE XREF: MidiAllNotesOff()+Dp
                    ; MidiMessage(x)+297p

bChannel    = byte ptr  4
a2      = byte ptr  8

        push    ebx
        mov bl, [esp+4+bChannel]
        push    esi
        push    edi
        xor edi, edi
        mov esi, offset _voice_table

loc_6BC05F22:               ; CODE XREF: note_off(x,x)+44j
        mov al, [esi]
        test    al, 1
        jz  short loc_6BC05F4E
        cmp [esi+4], bl
        jnz short loc_6BC05F4E
        mov cl, [esi+5]
        cmp cl, [esp+0Ch+a2]
        jnz short loc_6BC05F4E
        movzx   ecx, bl
        test    _hold_table[ecx], 1
        jz  short loc_6BC05F48
        or  al, 4
        mov [esi], al
        jmp short loc_6BC05F4E
; ---------------------------------------------------------------------------

loc_6BC05F48:               ; CODE XREF: note_off(x,x)+2Cj
        push    edi     ; a2
        call    _voice_off@4    ; voice_off(x)

loc_6BC05F4E:               ; CODE XREF: note_off(x,x)+12j
                    ; note_off(x,x)+17j ...
        add esi, 1Ah
        inc edi
        cmp esi, (offset _voice_table.flags1+1D4h)
        jl  short loc_6BC05F22
        pop edi
        pop esi
        pop ebx
        retn    8
ZZZ_note_off@8 endp






; char *__stdcall note_on(unsigned __int8 channel, unsigned __int8 a2, unsigned __int8 a3)
_note_on@12 proc near       ; CODE XREF: MidiMessage(x)+28Ap

a2      = dword ptr -4
channel     = byte ptr  8
a6      = dword ptr  0Ch
arg_8       = byte ptr  10h

        push    ebp
        mov ebp, esp
        push    ecx
        cmp [ebp+channel], 9
        push    ebx
        mov bl, byte ptr [ebp+a6]
        push    esi
        push    edi
        jnz short loc_6BC06A0F
        movzx   ecx, bl
        add ecx, 80h ; '€'
        jmp short loc_6BC06A1A
; ---------------------------------------------------------------------------

loc_6BC06A0F:               ; CODE XREF: note_on(x,x,x)+Ej
        movzx   eax, [ebp+channel]
        movzx   ecx, _program_table[eax]

loc_6BC06A1A:               ; CODE XREF: note_on(x,x,x)+19j
        mov eax, _gBankMem
        movzx   esi, byte ptr [eax+ecx*2+1]
        lea ecx, [eax+ecx*2]
        shl esi, 8
        movzx   ecx, byte ptr [ecx]
        add esi, ecx
        jz  loc_6BC06BFE
        lea ecx, [esi+eax]
        movzx   eax, byte ptr [esi+eax]
        mov edx, eax
        shr edx, 1
        and edx, 3
        sub edx, 0
        jz  loc_6BC06B84
        dec edx
        jz  loc_6BC06AF7
        dec edx
        jnz loc_6BC06BD3
        movzx   edx, [ebp+channel]
        mov cl, [ecx+24h]
        and eax, 1
        movzx   ebx, bl
        push    ebx     ; a4
        and ecx, 1
        push    edx     ; channel
        push    ecx     ; a2
        push    eax     ; a1
        mov [ebp+a6], edx
        call    _find_voice@16  ; find_voice(x,x,x,x)
        mov edi, 0FFh
        cmp _v1, edi
        jnz short loc_6BC06A99
        mov eax, _gBankMem
        mov al, [esi+eax]
        and eax, 1
        push    eax     ; a1
        call    _steal_voice@4  ; steal_voice(x)
        mov _v1, eax

loc_6BC06A99:               ; CODE XREF: note_on(x,x,x)+8Dj
        cmp _v2, edi
        jnz short loc_6BC06AB8
        mov eax, _gBankMem
        mov al, [esi+eax+24h]
        and eax, 1
        push    eax     ; a1
        call    _steal_voice@4  ; steal_voice(x)
        mov _v2, eax

loc_6BC06AB8:               ; CODE XREF: note_on(x,x,x)+ABj
        movzx   edi, [ebp+arg_8]
        push    edi     ; a2
        push    ebx     ; a1
        push    [ebp+a6]    ; a6
        push    esi     ; a3
        push    _v1   ; voiceNr
        call    _setup_voice@20 ; setup_voice(x,x,x,x,x)
        push    edi     ; a2
        push    ebx     ; a1
        push    [ebp+a6]    ; a6
        add esi, 24h ; '$'
        push    esi     ; a3
        push    _v2   ; voiceNr
        call    _setup_voice@20 ; setup_voice(x,x,x,x,x)
        push    _v1   ; a2
        call    _voice_on@4 ; voice_on(x)
        push    _v2
        jmp loc_6BC06BCE
; ---------------------------------------------------------------------------

loc_6BC06AF7:               ; CODE XREF: note_on(x,x,x)+59j
        movzx   edx, [ebp+channel]
        mov cl, [ecx+24h]
        and eax, 1
        movzx   ebx, bl
        push    ebx     ; a4
        and ecx, 1
        push    edx     ; channel
        push    ecx     ; a2
        push    eax     ; a1
        mov [ebp+a6], edx
        call    _find_voice@16  ; find_voice(x,x,x,x)
        mov eax, _v1
        mov edi, 0FFh
        cmp eax, edi
        jnz short loc_6BC06B37
        mov eax, _gBankMem
        mov al, [esi+eax]
        and eax, 1
        push    eax     ; a1
        call    _steal_voice@4  ; steal_voice(x)
        mov _v1, eax

loc_6BC06B37:               ; CODE XREF: note_on(x,x,x)+12Bj
        movzx   ecx, [ebp+arg_8]
        push    ecx     ; a2
        push    ebx     ; a1
        push    [ebp+a6]    ; a6
        mov [ebp+a2], ecx
        push    esi     ; a3
        push    eax     ; voiceNr
        call    _setup_voice@20 ; setup_voice(x,x,x,x,x)
        mov eax, _v2
        cmp eax, edi
        jz  short loc_6BC06BC8
        push    [ebp+a2]    ; a2
        add esi, 24h ; '$'
        push    ebx     ; a1
        push    [ebp+a6]    ; a6
        push    esi     ; a3
        push    eax     ; voiceNr
        call    _setup_voice@20 ; setup_voice(x,x,x,x,x)
        mov ecx, _v2
        mov eax, ecx
        push    ecx     ; a2
        imul    eax, 1Ah
        or  byte ptr _voice_table.flags1[eax], 8
        lea eax, _voice_table.flags1[eax]
        call    _voice_on@4 ; voice_on(x)
        jmp short loc_6BC06BC8
; ---------------------------------------------------------------------------

loc_6BC06B84:               ; CODE XREF: note_on(x,x,x)+52j
        movzx   edi, bl
        movzx   ebx, [ebp+channel]
        push    edi     ; a4
        push    ebx     ; channel
        and eax, 1
        push    0       ; a2
        push    eax     ; a1
        call    _find_voice@16  ; find_voice(x,x,x,x)
        mov eax, _v1
        cmp eax, 0FFh
        jnz short loc_6BC06BBA
        mov eax, _gBankMem
        mov al, [esi+eax]
        and eax, 1
        push    eax     ; a1
        call    _steal_voice@4  ; steal_voice(x)
        mov _v1, eax

loc_6BC06BBA:               ; CODE XREF: note_on(x,x,x)+1AEj
        movzx   ecx, [ebp+arg_8]
        push    ecx     ; a2
        push    edi     ; a1
        push    ebx     ; a6
        push    esi     ; a3
        push    eax     ; voiceNr
        call    _setup_voice@20 ; setup_voice(x,x,x,x,x)

loc_6BC06BC8:               ; CODE XREF: note_on(x,x,x)+15Dj
                    ; note_on(x,x,x)+18Ej
        push    _v1   ; a2

loc_6BC06BCE:               ; CODE XREF: note_on(x,x,x)+FEj
        call    _voice_on@4 ; voice_on(x)

loc_6BC06BD3:               ; CODE XREF: note_on(x,x,x)+60j
        movzx   eax, [ebp+channel]
        mov cl, [ebp+arg_8]
        mov _gbVelLevel[eax], cl
        inc _note_offs[eax]
        movzx   ecx, _note_offs[eax]
        lea eax, _note_offs[eax]
        cmp ecx, 100h
        jnz short loc_6BC06BFE
        and byte ptr [eax], 0

loc_6BC06BFE:               ; CODE XREF: note_on(x,x,x)+3Bj
                    ; note_on(x,x,x)+205j
        pop edi
        pop esi
        pop ebx
        leave
        retn    0Ch
_note_on@12 endp





; void __stdcall setup_voice(int voiceNr, int a3, int a6, int a1, signed int a2)
ZZZ_setup_voice@20 proc near       ; CODE XREF: note_on(x,x,x)+D4p
                    ; note_on(x,x,x)+E8p ...

var_8       = dword ptr -8
var_1       = byte ptr -1
voiceNr     = dword ptr  8
a3      = dword ptr  0Ch
a6      = dword ptr  10h
a1      = dword ptr  14h
a2      = dword ptr  18h

        push    ebp
        mov ebp, esp
        push    ecx
        push    ecx     ; a9
        mov eax, _gBankMem
        push    ebx     ; a9
        push    esi     ; a9
        push    edi     ; a9
        mov edi, [ebp+a3]
        push    [ebp+voiceNr]   ; a9
        add eax, edi
        mov esi, [ebp+voiceNr]
        push    0       ; a8
        add edi, 4
        mov cl, [eax]
        mov al, [eax+3]
        push    [ebp+a6]    ; a7
        mov [ebp+var_1], al
        movzx   ebx, al
        movzx   eax, cl
        mov byte ptr [ebp+a3], cl
        mov ecx, ebx
        mov [ebp+var_8], eax
        and ecx, 3
        and eax, 10h
        push    ecx     ; a6
        shl esi, 5
        push    eax     ; a5
        push    esi     ; a4
        push    [ebp+a2]    ; a3
        push    [ebp+a1]    ; a2
        push    edi     ; a1
        call    _setup_operator@36 ; setup_operator(x,x,x,x,x,x,x,x,x)
        push    [ebp+voiceNr]   ; a9
        mov eax, ebx
        shr eax, 2
        push    1       ; a8
        and eax, 3
        push    [ebp+a6]    ; a7
        push    eax     ; a6
        mov eax, [ebp+var_8]
        and eax, 20h
        push    eax     ; a5
        lea eax, [esi+8]
        push    eax     ; a4
        lea eax, [edi+8]
        push    [ebp+a2]    ; a3
        push    [ebp+a1]    ; a2
        push    eax     ; a1
        call    _setup_operator@36 ; setup_operator(x,x,x,x,x,x,x,x,x)
        push    [ebp+voiceNr]   ; a9
        mov eax, ebx
        shr eax, 4
        push    2       ; a8
        and eax, 3
        push    [ebp+a6]    ; a7
        push    eax     ; a6
        mov eax, [ebp+var_8]
        and eax, 40h
        push    eax     ; a5
        lea eax, [esi+10h]
        push    eax     ; a4
        lea eax, [edi+10h]
        push    [ebp+a2]    ; a3
        push    [ebp+a1]    ; a2
        push    eax     ; a1
        call    _setup_operator@36 ; setup_operator(x,x,x,x,x,x,x,x,x)
        push    [ebp+voiceNr]   ; a9
        mov eax, [ebp+var_8]
        shr ebx, 6
        push    3       ; a8
        and eax, 80h
        push    [ebp+a6]    ; a7
        add esi, 18h
        add edi, 18h
        push    ebx     ; a6
        mov ebx, [ebp+a1]
        push    eax     ; a5
        push    esi     ; a4
        push    [ebp+a2]    ; a3
        push    ebx     ; a2
        push    edi     ; a1
        call    _setup_operator@36 ; setup_operator(x,x,x,x,x,x,x,x,x)
        mov eax, [ebp+voiceNr]
        imul    eax, 1Ah
        mov cl, byte ptr [ebp+a3]
        pop edi
        mov _voice_table.field_10[eax], cl
        mov cl, [ebp+var_1]
        mov _voice_table.flags2[eax], cl
        mov cx, word ptr _gwTimer
        inc word ptr _gwTimer
        mov byte ptr _voice_table.flags1[eax], 1
        mov _voice_table.timer[eax], cx
        mov cl, byte ptr [ebp+a6]
        mov _voice_table.channel[eax], cl
        mov _voice_table.field_5[eax], bl
        pop esi
        pop ebx
        leave
        retn    14h
ZZZ_setup_voice@20 endp




; int __stdcall find_voice(int a1, int a2, int channel, int a4)
_find_voice@16  proc near       ; CODE XREF: note_on(x,x,x)+7Dp
                    ; note_on(x,x,x)+11Ap ...

var_4       = dword ptr -4
a1      = dword ptr  8
a2      = dword ptr  0Ch
channel     = dword ptr  10h
a4      = dword ptr  14h

        push    ebp
        mov ebp, esp
        push    ecx
        push    ebx
        push    esi
        push    edi
        mov eax, 0FFh
        xor edi, edi
        mov _v1, eax
        mov _v2, eax
        xor ebx, ebx
        mov [ebp+var_4], edi
        mov esi, offset _voice_table

loc_6BC05F82:               ; CODE XREF: find_voice(x,x,x,x)+85j
        test    byte ptr [esi], 1
        jz  short loc_6BC05FA4
        movzx   eax, byte ptr [esi+4]
        cmp eax, [ebp+channel]
        jnz short loc_6BC05F9F
        movzx   eax, byte ptr [esi+5]
        cmp eax, [ebp+a4]
        jnz short loc_6BC05F9F
        push    edi     ; a2
        call    _voice_off@4    ; voice_off(x)

loc_6BC05F9F:               ; CODE XREF: find_voice(x,x,x,x)+2Ej
                    ; find_voice(x,x,x,x)+37j
        test    byte ptr [esi], 1
        jnz short loc_6BC05FDB

loc_6BC05FA4:               ; CODE XREF: find_voice(x,x,x,x)+25j
        mov ax, word ptr _gwTimer
        sub ax, [esi+2]
        cmp ax, bx
        jb  short loc_6BC05FCC
        mov ecx, _v1
        mov [ebp+var_4], ebx
        mov _v2, ecx
        mov ebx, eax
        mov _v1, edi
        jmp short loc_6BC05FDB
; ---------------------------------------------------------------------------

loc_6BC05FCC:               ; CODE XREF: find_voice(x,x,x,x)+51j
        cmp ax, word ptr [ebp+var_4]
        jb  short loc_6BC05FDB
        mov [ebp+var_4], eax
        mov _v2, edi

loc_6BC05FDB:               ; CODE XREF: find_voice(x,x,x,x)+42j
                    ; find_voice(x,x,x,x)+6Aj ...
        add esi, 1Ah
        inc edi
        cmp esi, (offset _voice_table.flags1+1A0h)
        jl  short loc_6BC05F82
        test    _voice_table.flags1+1A0h, 1
        push    10h
        pop edi
        jz  short loc_6BC0601A
        movzx   eax, _voice_table.channel+1A0h
        cmp eax, [ebp+channel]
        jnz short loc_6BC06011
        movzx   eax, _voice_table.field_5+1A0h
        cmp eax, [ebp+a4]
        jnz short loc_6BC06011
        push    edi     ; a2
        call    _voice_off@4    ; voice_off(x)

loc_6BC06011:               ; CODE XREF: find_voice(x,x,x,x)+9Dj
                    ; find_voice(x,x,x,x)+A9j
        test    _voice_table.flags1+1A0h, 1
        jnz short loc_6BC0605E

loc_6BC0601A:               ; CODE XREF: find_voice(x,x,x,x)+91j
        mov eax, _gwTimer
        sub eax, dword ptr _voice_table.timer+1A0h
        cmp [ebp+a1], 0
        jnz short loc_6BC06049
        cmp ax, bx
        jb  short loc_6BC06049
        mov ecx, _v1
        mov [ebp+var_4], ebx
        mov _v2, ecx
        mov ebx, eax
        mov _v1, edi
        jmp short loc_6BC0605E
; ---------------------------------------------------------------------------

loc_6BC06049:               ; CODE XREF: find_voice(x,x,x,x)+C9j
                    ; find_voice(x,x,x,x)+CEj
        cmp [ebp+a2], 0
        jnz short loc_6BC0605E
        cmp ax, word ptr [ebp+var_4]
        jb  short loc_6BC0605E
        mov [ebp+var_4], eax
        mov _v2, edi

loc_6BC0605E:               ; CODE XREF: find_voice(x,x,x,x)+B8j
                    ; find_voice(x,x,x,x)+E7j ...
        test    _voice_table.flags1+1BAh, 1
        push    11h
        pop esi
        jz  short loc_6BC06091
        movzx   eax, _voice_table.channel+1BAh
        cmp eax, [ebp+channel]
        jnz short loc_6BC06088
        movzx   eax, _voice_table.field_5+1BAh
        cmp eax, [ebp+a4]
        jnz short loc_6BC06088
        push    esi     ; a2
        call    _voice_off@4    ; voice_off(x)

loc_6BC06088:               ; CODE XREF: find_voice(x,x,x,x)+114j
                    ; find_voice(x,x,x,x)+120j
        test    _voice_table.flags1+1BAh, 1
        jnz short loc_6BC060D4

loc_6BC06091:               ; CODE XREF: find_voice(x,x,x,x)+108j
        mov eax, _gwTimer
        xor ecx, ecx
        sub eax, dword ptr _voice_table.timer+1BAh
        cmp [ebp+a1], ecx
        jnz short loc_6BC060C3
        cmp ax, bx
        jb  short loc_6BC060C3
        mov eax, _v1
        cmp eax, edi
        jnz short loc_6BC060B6
        cmp [ebp+a2], ecx
        jnz short loc_6BC060BB

loc_6BC060B6:               ; CODE XREF: find_voice(x,x,x,x)+14Fj
        mov _v2, eax

loc_6BC060BB:               ; CODE XREF: find_voice(x,x,x,x)+154j
        mov _v1, esi
        jmp short loc_6BC060D4
; ---------------------------------------------------------------------------

loc_6BC060C3:               ; CODE XREF: find_voice(x,x,x,x)+141j
                    ; find_voice(x,x,x,x)+146j
        cmp [ebp+a2], ecx
        jnz short loc_6BC060D4
        cmp ax, word ptr [ebp+var_4]
        jb  short loc_6BC060D4
        mov _v2, esi

loc_6BC060D4:               ; CODE XREF: find_voice(x,x,x,x)+12Fj
                    ; find_voice(x,x,x,x)+161j ...
        pop edi
        pop esi
        pop ebx
        leave
        retn    10h
_find_voice@16  endp





; int __stdcall steal_voice(int a1)
_steal_voice@4  proc near       ; CODE XREF: note_on(x,x,x)+9Bp
                    ; note_on(x,x,x)+BAp ...

var_10      = dword ptr -10h
var_C       = dword ptr -0Ch
a2      = dword ptr -8
var_1       = byte ptr -1
a1      = dword ptr  8

        push    ebp
        mov ebp, esp
        sub esp, 10h
        mov eax, [ebp+a1]
        push    edi
        neg eax
        sbb eax, eax
        and byte ptr [ebp+a1], 0
        and [ebp+var_1], 0
        and al, 0FEh
        add eax, 12h
        xor edi, edi
        test    eax, eax
        jle short loc_6BC0615E
        mov edx, offset _voice_table.channel
        push    esi
        mov [ebp+var_10], edx
        jmp short loc_6BC0610B
; ---------------------------------------------------------------------------

loc_6BC06108:               ; CODE XREF: steal_voice(x)+7Fj
        mov edx, [ebp+var_10]

loc_6BC0610B:               ; CODE XREF: steal_voice(x)+2Aj
        mov si, word ptr _gwTimer
        mov cl, [edx]
        sub si, [edx-2]
        cmp cl, 9
        jnz short loc_6BC06122
        add cl, 0F8h ; 'ø'
        jmp short loc_6BC06125
; ---------------------------------------------------------------------------

loc_6BC06122:               ; CODE XREF: steal_voice(x)+3Fj
        add cl, 2

loc_6BC06125:               ; CODE XREF: steal_voice(x)+44j
        mov dl, [edx-4]
        and dl, 8
        cmp [ebp+var_1], dl
        jz  short loc_6BC0613C
        cmp [ebp+var_1], 0
        jnz short loc_6BC06154
        mov [ebp+var_1], 8
        jmp short loc_6BC06141
; ---------------------------------------------------------------------------

loc_6BC0613C:               ; CODE XREF: steal_voice(x)+52j
        cmp cl, byte ptr [ebp+a1]
        jbe short loc_6BC06146

loc_6BC06141:               ; CODE XREF: steal_voice(x)+5Ej
        mov byte ptr [ebp+a1], cl
        jmp short loc_6BC0614E
; ---------------------------------------------------------------------------

loc_6BC06146:               ; CODE XREF: steal_voice(x)+63j
        jnz short loc_6BC06154
        cmp si, word ptr [ebp+var_C]
        jbe short loc_6BC06154

loc_6BC0614E:               ; CODE XREF: steal_voice(x)+68j
        mov [ebp+var_C], esi
        mov [ebp+a2], edi

loc_6BC06154:               ; CODE XREF: steal_voice(x)+58j
                    ; steal_voice(x):loc_6BC06146j ...
        add [ebp+var_10], 1Ah
        inc edi
        cmp edi, eax
        jl  short loc_6BC06108
        pop esi

loc_6BC0615E:               ; CODE XREF: steal_voice(x)+1Fj
        push    [ebp+a2]    ; a2
        call    _voice_off@4    ; voice_off(x)
        mov eax, [ebp+a2]
        pop edi
        leave
        retn    4
_steal_voice@4  endp






; int __stdcall setup_operator(DWORD a1, int a2, signed int a3, signed int a4, int a5, int a6, int a7, int a8, int voiceNr)
_setup_operator@36 proc near        ; CODE XREF: setup_voice(x,x,x,x,x)+49p
                    ; setup_voice(x,x,x,x,x)+74p ...

var_C       = word ptr -0Ch
var_8       = dword ptr -8
var_4       = dword ptr -4
a0      = dword ptr  8
a2      = dword ptr  0Ch
a3      = dword ptr  10h
a1      = word ptr  14h
a4      = dword ptr  18h
a5      = dword ptr  1Ch
a6      = dword ptr  20h
a7      = dword ptr  24h
voiceNr     = dword ptr  28h
a9      = dword ptr  2Ch

        push    ebp
        mov ebp, esp
        sub esp, 0Ch
        mov eax, [ebp+a6]
        push    ebx
        push    esi
        mov esi, dword ptr [ebp+a1]
        movzx   eax, _pan_mask[eax]
        mov [ebp+var_8], eax
        push    edi
        lea eax, [esi+7]
        push    0       ; a2
        push    eax     ; a1
        mov dword ptr [ebp+var_C], eax
        call    _fmwrite231@8   ; fmwrite(x,x)
        cmp [ebp+a4], 0
        mov edi, _gBankMem
        mov ebx, [ebp+a0]
        jnz short loc_6BC06698
        mov cl, [ebx+edi+5]
        mov al, [ebx+edi+4]
        and ecx, 1Fh
        and eax, 3
        shl ecx, 2
        or  eax, ecx
        test    al, 40h
        jz  short loc_6BC06695
        or  ax, 0FF80h

loc_6BC06695:               ; CODE XREF: setup_operator(x,x,x,x,x,x,x,x,x)+4Bj
        add [ebp+a2], eax

loc_6BC06698:               ; CODE XREF: setup_operator(x,x,x,x,x,x,x,x,x)+34j
        mov ecx, [ebp+a2]
        cmp ecx, 13h
        jge short loc_6BC066B8
        push    1Eh
        xor edx, edx
        pop eax
        push    0Ch
        sub eax, ecx
        pop ecx
        div ecx
        mov ecx, [ebp+a2]
        lea eax, [eax+eax*2]
        lea ecx, [ecx+eax*4]
        mov [ebp+a2], ecx

loc_6BC066B8:               ; CODE XREF: setup_operator(x,x,x,x,x,x,x,x,x)+5Aj
        cmp ecx, 72h ; 'r'
        jle short loc_6BC066D3
        push    0Ch
        lea eax, [ecx-67h]
        xor edx, edx
        pop ecx
        div ecx

loc_6BC066C7:               ; CODE XREF: setup_operator(x,x,x,x,x,x,x,x,x)+8Dj
        mov ecx, [ebp+a2]
        sub ecx, 0Ch
        dec eax
        mov [ebp+a2], ecx
        jnz short loc_6BC066C7

loc_6BC066D3:               ; CODE XREF: setup_operator(x,x,x,x,x,x,x,x,x)+77j
        lea eax, [ecx-13h]
        push    0Ch
        mov [ebp+a2], eax
        pop ecx     ; bytesWritten
        cdq
        idiv    ecx
        mov eax, [ebp+a2]
        mov [ebp+var_4], edx
        cdq
        idiv    ecx
        mov [ebp+a2], eax
        movzx   eax, byte ptr [ebx+edi]
        push    eax     ; a2
        push    esi     ; a1
        call    _fmwrite231@8   ; fmwrite(x,x)
        mov eax, [ebp+a5]
        xor ecx, ecx
        push    3Fh ; '?'
        sub eax, ecx
        pop edi
        jz  short loc_6BC06741
        dec eax
        jz  short loc_6BC06733
        dec eax
        jz  short loc_6BC06728
        dec eax
        jnz short loc_6BC0673E
        cmp [ebp+a3], 40h ; '@'
        jl  short loc_6BC0671C
        push    7Fh ; ''
        pop ecx
        sub ecx, [ebp+a3]
        sar ecx, 2
        jmp short loc_6BC06741
; ---------------------------------------------------------------------------

loc_6BC0671C:               ; CODE XREF: setup_operator(x,x,x,x,x,x,x,x,x)+CBj
        mov ecx, edi
        sub ecx, [ebp+a3]
        sar ecx, 1
        add ecx, 10h
        jmp short loc_6BC06741
; ---------------------------------------------------------------------------

loc_6BC06728:               ; CODE XREF: setup_operator(x,x,x,x,x,x,x,x,x)+C2j
        push    7Fh ; ''
        pop ecx
        sub ecx, [ebp+a3]
        sar ecx, 3
        jmp short loc_6BC06741
; ---------------------------------------------------------------------------

loc_6BC06733:               ; CODE XREF: setup_operator(x,x,x,x,x,x,x,x,x)+BFj
        push    7Fh ; ''
        pop ecx
        sub ecx, [ebp+a3]
        sar ecx, 4
        jmp short loc_6BC06741
; ---------------------------------------------------------------------------

loc_6BC0673E:               ; CODE XREF: setup_operator(x,x,x,x,x,x,x,x,x)+C5j
        mov ecx, [ebp+a2]

loc_6BC06741:               ; CODE XREF: setup_operator(x,x,x,x,x,x,x,x,x)+BCj
                    ; setup_operator(x,x,x,x,x,x,x,x,x)+D6j ...
        mov eax, _gBankMem
        movzx   eax, byte ptr [ebx+eax+1]
        mov edx, eax
        and edx, edi
        add edx, ecx
        cmp edx, edi
        jle short loc_6BC06757
        mov edx, edi

loc_6BC06757:               ; CODE XREF: setup_operator(x,x,x,x,x,x,x,x,x)+10Fj
        mov edi, [ebp+voiceNr]
        push    [ebp+a6]    ; a3
        mov ecx, edi
        and eax, 0C0h
        imul    ecx, 1Ah
        push    [ebp+a5]    ; a2
        add eax, edx
        mov edx, [ebp+a7]
        add ecx, edx
        push    eax     ; a1
        mov [ebp+a3], ecx
        mov _voice_table.field_15[ecx], al
        call    _NATV_CalcVolume@12 ; NATV_CalcVolume(x,x,x)
        movzx   eax, al
        push    eax     ; a2
        lea eax, [esi+1]
        push    eax     ; a1
        call    _fmwrite231@8   ; fmwrite(x,x)
        mov eax, _gBankMem
        movzx   eax, byte ptr [ebx+eax+2]
        push    eax     ; a2
        lea eax, [esi+2]
        push    eax     ; a1
        call    _fmwrite231@8   ; fmwrite(x,x)
        mov eax, _gBankMem
        movzx   eax, byte ptr [ebx+eax+3]
        push    eax     ; a2
        lea eax, [esi+3]
        push    eax     ; a1
        call    _fmwrite231@8   ; fmwrite(x,x)
        cmp [ebp+a4], 0
        mov eax, _gBankMem
        jnz loc_6BC06874
        movsx   edx, byte ptr [eax+ebx+4]
        add eax, ebx
        and edx, 0FFFFFFFCh
        jz  short loc_6BC067F4
        mov ecx, [ebp+var_4]
        mov ebx, [ebp+a2]
        sar edx, 2
        mov ecx, _td_adjust_setup_operator[ecx*4]
        imul    ecx, edx
        sar ecx, 8
        cmp ebx, 1
        mov edx, ecx
        jle short loc_6BC067F7
        lea ecx, [ebx-1]
        sar edx, cl
        jmp short loc_6BC067F7
; ---------------------------------------------------------------------------

loc_6BC067F4:               ; CODE XREF: setup_operator(x,x,x,x,x,x,x,x,x)+18Aj
        mov ebx, [ebp+a2]

loc_6BC067F7:               ; CODE XREF: setup_operator(x,x,x,x,x,x,x,x,x)+1A7j
                    ; setup_operator(x,x,x,x,x,x,x,x,x)+1AEj
        mov ecx, [ebp+var_4]
        mov al, [eax+5]
        and eax, 0E0h
        movzx   esi, _fnum[ecx*2]
        add esi, edx
        mov ecx, esi
        sar ecx, 8
        and ecx, 3
        lea ebx, [ecx+ebx*4]
        add ebx, eax
        mov eax, [ebp+a3]
        mov _voice_table.field_11[eax], bl
        mov eax, [ebp+a6]
        movzx   cx, byte ptr _gbChanBendRange[eax]
        mov ax, _giBend[eax*2]
        push    ecx     ; a3
        push    eax     ; iBend
        push    esi     ; a1
        call    _NATV_CalcBend@12 ; NATV_CalcBend(x,x,x)
        push    [ebp+a2]
        movzx   eax, ax
        push    eax
        call    _MidiCalcFAndB@8 ; MidiCalcFAndB(x,x)
        imul    edi, 0Dh
        xor edx, edx
        and ebx, 0E0h
        mov dl, ah
        movzx   ecx, al
        movzx   eax, dl
        add ebx, eax
        add edi, [ebp+a7]
        mov [ebp+a2], ebx
        mov ebx, [ebp+a0]
        mov _voice_table.field_8[edi*2], si
        mov esi, dword ptr [ebp+a1]
        jmp short loc_6BC06882
; ---------------------------------------------------------------------------

loc_6BC06874:               ; CODE XREF: setup_operator(x,x,x,x,x,x,x,x,x)+17Aj
        movzx   ecx, byte ptr [eax+ebx+4] ; bytesWritten
        add eax, ebx
        movzx   eax, byte ptr [eax+5]
        mov [ebp+a2], eax

loc_6BC06882:               ; CODE XREF: setup_operator(x,x,x,x,x,x,x,x,x)+22Ej
        lea eax, [esi+4]
        push    ecx     ; a2
        push    eax     ; a1
        call    _fmwrite231@8   ; fmwrite(x,x)
        push    [ebp+a2]    ; a2
        lea eax, [esi+5]
        push    eax     ; a1
        call    _fmwrite231@8   ; fmwrite(x,x)
        mov eax, _gBankMem
        mov al, [ebx+eax+6]
        test    al, 30h
        jz  short loc_6BC068B5
        cmp [ebp+var_8], 30h ; '0'
        jz  short loc_6BC068B5
        and eax, 0CFh
        or  eax, [ebp+var_8]
        jmp short loc_6BC068B8
; ---------------------------------------------------------------------------

loc_6BC068B5:               ; CODE XREF: setup_operator(x,x,x,x,x,x,x,x,x)+25Fj
                    ; setup_operator(x,x,x,x,x,x,x,x,x)+265j
        movzx   eax, al

loc_6BC068B8:               ; CODE XREF: setup_operator(x,x,x,x,x,x,x,x,x)+26Fj
        add esi, 6
        push    eax     ; a2
        push    esi     ; a1
        call    _fmwrite231@8   ; fmwrite(x,x)
        mov eax, _gBankMem
        movzx   eax, byte ptr [ebx+eax+7]
        push    eax     ; a2
        push    dword ptr [ebp+var_C] ; a1
        call    _fmwrite231@8   ; fmwrite(x,x)
        pop edi
        pop esi
        pop ebx
        leave
        retn    24h
_setup_operator@36 endp





; void __stdcall voice_on(signed int voiceNr)
ZZZ_voice_on@4 proc near       ; CODE XREF: note_on(x,x,x)+F3p
                    ; note_on(x,x,x)+189p ...

voiceNr     = dword ptr  4

        mov eax, [esp+voiceNr]
        push    1       ; a2
        cmp eax, 10h
        jge short loc_6BC06181
        add eax, 240h
        push    eax
        jmp short loc_6BC061A7
; ---------------------------------------------------------------------------

loc_6BC06181:               ; CODE XREF: voice_on(x)+9j
        jnz short loc_6BC06196
        push    250h        ; a1
        call    _fmwrite231@8   ; fmwrite(x,x)
        push    1
        push    251h
        jmp short loc_6BC061A7
; ---------------------------------------------------------------------------

loc_6BC06196:               ; CODE XREF: voice_on(x):loc_6BC06181j
        push    252h        ; a1
        call    _fmwrite231@8   ; fmwrite(x,x)
        push    1       ; a2
        push    253h        ; a1

loc_6BC061A7:               ; CODE XREF: voice_on(x)+11j
                    ; voice_on(x)+26j
        call    _fmwrite231@8   ; fmwrite(x,x)
        retn    4
ZZZ_voice_on@4 endp







; void __stdcall hold_controller(int a2, signed int a3)
_hold_controller@8 proc near        ; CODE XREF: MidiMessage(x)+139p

a2      = dword ptr  4
a3      = dword ptr  8

        cmp [esp+a3], 40h ; '@'
        jl  short loc_6BC06C1A
        mov eax, [esp+a2]
        or  _hold_table[eax], 1
        jmp short locret_6BC06C51
; ---------------------------------------------------------------------------

loc_6BC06C1A:               ; CODE XREF: hold_controller(x,x)+5j
        push    edi
        push    esi
        mov esi, [esp+8+a2]
        push    ebx
        mov edi, offset _voice_table.channel
        and _hold_table[esi], 0FEh
        xor ebx, ebx

loc_6BC06C2F:               ; CODE XREF: hold_controller(x,x)+46j
        test    byte ptr [edi-4], 4
        jz  short loc_6BC06C42
        movzx   eax, byte ptr [edi]
        cmp eax, esi
        jnz short loc_6BC06C42
        push    ebx     ; a2
        call    _voice_off@4    ; voice_off(x)

loc_6BC06C42:               ; CODE XREF: hold_controller(x,x)+2Dj
                    ; hold_controller(x,x)+34j
        add edi, 1Ah
        inc ebx
        cmp edi, (offset _voice_table.channel+1D4h)
        jl  short loc_6BC06C2F
        pop ebx
        pop esi
        pop edi

locret_6BC06C51:            ; CODE XREF: hold_controller(x,x)+12j
        retn    8
_hold_controller@8 endp






; void __stdcall fmreset()
_fmreset@0  proc near       ; CODE XREF: MidiOpenDevice(x,x)+50p
                    ; MidiOpen()+10p ...
;       mov eax, _MidiPosition
;       add _MidiPosition, 2
;       shl eax, 2
        push    edi
;       mov _DeviceData[eax], 38Ah
;       mov (_DeviceData+2)[eax], 5
;       mov (_DeviceData+4)[eax], 389h
;       mov (_DeviceData+6)[eax], 80h ; '€'
;       call    _MidiFlush@0    ; MidiFlush()
        push    5
        push    80h
        call    _fmwrite21@8
        push    8
        mov eax, 20002000h
        pop ecx
        mov edi, offset _giBend
        rep stosd
        mov eax, 2020202h
        mov edi, offset _gbChanBendRange
        stosd
        stosd
        stosd
        stosd
        xor eax, eax
        mov edi, offset _hold_table
        stosd
        stosd
        stosd
        stosd
        mov eax, 7F7F7F7Fh
        mov edi, offset _gbChanExpr
        stosd
        stosd
        stosd
        stosd
        mov eax, 64646464h
        mov edi, offset _gbChanVolume
        stosd
        stosd
        stosd
        stosd
        mov eax, 4040404h
        mov edi, offset _gbChanAtten
        stosd
        stosd
        stosd
        stosd
        mov eax, 30303030h
        mov edi, offset _pan_mask
        stosd
        stosd
        stosd
        stosd
        mov eax, offset _voice_table
        xor ecx, ecx

loc_6BC05E74:               ; CODE XREF: fmreset()+AEj
        mov [eax+2], cx
        mov [eax], cl
        add eax, 1Ah
        cmp eax, (offset _voice_table.flags1+1D4h)
        jl  short loc_6BC05E74
        mov word ptr _gwTimer, cx
        pop edi
        retn
_fmreset@0  endp






; void __stdcall MidiAllNotesOff()
_MidiAllNotesOff@0 proc near        ; CODE XREF: MidiClose()+9p
        push    esi
        mov esi, offset _voice_table.field_5

loc_6BC05E94:               ; CODE XREF: MidiAllNotesOff()+1Bj
        mov al, [esi-1]
        push    eax     ; a2
        mov al, [esi]
        push    eax     ; bChannel
        call    _note_off@8 ; note_off(x,x)
        add esi, 1Ah
        cmp esi, (offset _voice_table.field_5+1D4h)
        jl  short loc_6BC05E94
        pop esi
        retn
_MidiAllNotesOff@0 endp


end