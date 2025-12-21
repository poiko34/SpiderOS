[BITS 32]

%define USER_CS ((4 << 3) | 3)   ; 0x23 (если у тебя так в GDT)
%define USER_DS ((5 << 3) | 3)   ; 0x2B

global enter_user_mode

; void enter_user_mode(uint32_t entry, uint32_t user_stack)
; entry      = [esp + 4]
; user_stack = [esp + 8]

enter_user_mode:
    cli

    ; сохраним аргументы
    mov eax, [esp + 4]   ; entry
    mov ebx, [esp + 8]   ; user_stack

    ; user data сегменты
    mov cx, USER_DS
    mov ds, cx
    mov es, cx
    mov fs, cx
    mov gs, cx

    ; iret frame
    push dword USER_DS   ; SS
    push dword ebx       ; ESP
    push dword 0x202     ; EFLAGS: IF=1, OF=0, DF=0, TF=0
    push dword USER_CS   ; CS
    push dword eax       ; EIP

    iretd
