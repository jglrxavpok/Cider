PUBLIC get_context
PUBLIC set_context
PUBLIC swap_context_on_top

.code
; Store the current context inside a given WinContext* (stored in rcx)
; RCX = WinContext* pCurrentContext
get_context PROC FRAME
    .endprolog
    ; at this point, pointer to WinContext is in RCX
    mov r11, [rsp] ; read return address
    mov [rcx + 8*0], r11 ; store return address (RIP) of previous frame

    mov r11, rsp
    add r11, 8
    mov [rcx + 8*1], r11 ; store stack pointer of previous frame

    ; store non volatile register
    mov [rcx + 8 * 2], r12
    mov [rcx + 8 * 3], r13
    mov [rcx + 8 * 4], r14
    mov [rcx + 8 * 5], r15
    mov [rcx + 8 * 6], rdi
    mov [rcx + 8 * 7], rsi
    mov [rcx + 8 * 8], rbx
    mov [rcx + 8 * 9], rbp

    movaps [rcx + 8 * 10 + 16 * 0], xmm6
    movaps [rcx + 8 * 10 + 16 * 1], xmm7
    movaps [rcx + 8 * 10 + 16 * 2], xmm8
    movaps [rcx + 8 * 10 + 16 * 3], xmm9
    movaps [rcx + 8 * 10 + 16 * 4], xmm10
    movaps [rcx + 8 * 10 + 16 * 5], xmm11
    movaps [rcx + 8 * 10 + 16 * 6], xmm12
    movaps [rcx + 8 * 10 + 16 * 7], xmm13
    movaps [rcx + 8 * 10 + 16 * 8], xmm14
    movaps [rcx + 8 * 10 + 16 * 9], xmm15

    ; TIB information
    ; stack low address (top of stack)
    mov r10, gs:[010h]
    mov [rcx + 8 * 10 + 16 * 10 + 8 * 0], r10

    ; stack high address (bottom of stack)
    mov r10, gs:[008h]
    mov [rcx + 8 * 10 + 16 * 10 + 8 * 1], r10

    ; deallocation stack address (top of stack - a few pages)
    mov r10, gs:[1478h]
    mov [rcx + 8 * 10 + 16 * 10 + 8 * 2 + 8 * 0], r10

    ; guaranteed stack bytes
    mov r10, gs:[1748h]
    mov [rcx + 8 * 10 + 16 * 10 + 8 * 2 + 8 * 1], r10

    ret
get_context ENDP

; Restores the context from a WinContext* (stored in rcx). Can be used to return to a context made with get_context, or
;  a brand new context
; RCX = WinContext* pContextToSwitchTo
set_context PROC FRAME
    .endprolog
    ; at this point, pointer to WinContext is in RCX
    mov r11, [rcx + 0 * 0] ; return address, will push later
    mov rsp, [rcx + 8 * 1]

    ; restore non volatile register
    mov r12, [rcx + 8 * 2]
    mov r13, [rcx + 8 * 3]
    mov r14, [rcx + 8 * 4]
    mov r15, [rcx + 8 * 5]
    mov rdi, [rcx + 8 * 6]
    mov rsi, [rcx + 8 * 7]
    mov rbx, [rcx + 8 * 8]
    mov rbp, [rcx + 8 * 9]

    movaps xmm6,  [rcx + 8 * 10 + 16 * 0]
    movaps xmm7,  [rcx + 8 * 10 + 16 * 1]
    movaps xmm8,  [rcx + 8 * 10 + 16 * 2]
    movaps xmm9,  [rcx + 8 * 10 + 16 * 3]
    movaps xmm10, [rcx + 8 * 10 + 16 * 4]
    movaps xmm11, [rcx + 8 * 10 + 16 * 5]
    movaps xmm12, [rcx + 8 * 10 + 16 * 6]
    movaps xmm13, [rcx + 8 * 10 + 16 * 7]
    movaps xmm14, [rcx + 8 * 10 + 16 * 8]
    movaps xmm15, [rcx + 8 * 10 + 16 * 9]

    ; restore TIB information
    ; stack low address (top of stack)
    mov r10, [rcx + 8 * 10 + 16 * 10 + 8 * 0]
    mov gs:[010h], r10

    ; stack high address (bottom of stack)
    mov r10, [rcx + 8 * 10 + 16 * 10 + 8 * 1]
    mov gs:[008h], r10

    ; deallocation stack address (top of stack - a few pages)
    mov r10, [rcx + 8 * 10 + 16 * 10 + 8 * 2 + 8 * 0]
    mov gs:[1478h], r10

    ; guaranteed stack bytes
    mov r10, [rcx + 8 * 10 + 16 * 10 + 8 * 2 + 8 * 1]
    mov gs:[1748h], r10

    jmp r11 ; go to return address of get_context
set_context ENDP

; Resumes the context by calling the given function in R8, with its first argument being what is in RDX
;  When returning, that function will return to the context inside RCX
; RCX = WinContext* context to switch to
; RDX  = Pointer to user data
; R8  = Address of function to resume on.
;       This function will get the pointer to the previous context (the one calling swap_context_on_top) and the pointer to user data (in this order)
swap_context_on_top PROC FRAME
    .endprolog

    ; R11= temporary variable
    ; R10= pointer to saved context

    pop r11 ; read return address

    sub rsp, 272 ; reserve some space for WinContext object (push WinContext)
    mov r10, rsp

    ; Store current context on stack
    mov [rsp + 8*0], r11 ; store return address (RIP) of previous frame
    mov [rsp + 8*1], rsp ; store stack pointer of previous frame

    ; store non volatile registers
    mov [rsp + 8 * 2], r12
    mov [rsp + 8 * 3], r13
    mov [rsp + 8 * 4], r14
    mov [rsp + 8 * 5], r15
    mov [rsp + 8 * 6], rdi
    mov [rsp + 8 * 7], rsi
    mov [rsp + 8 * 8], rbx
    mov [rsp + 8 * 9], rbp

    movaps [rsp + 8 * 10 + 16 * 0], xmm6
    movaps [rsp + 8 * 10 + 16 * 1], xmm7
    movaps [rsp + 8 * 10 + 16 * 2], xmm8
    movaps [rsp + 8 * 10 + 16 * 3], xmm9
    movaps [rsp + 8 * 10 + 16 * 4], xmm10
    movaps [rsp + 8 * 10 + 16 * 5], xmm11
    movaps [rsp + 8 * 10 + 16 * 6], xmm12
    movaps [rsp + 8 * 10 + 16 * 7], xmm13
    movaps [rsp + 8 * 10 + 16 * 8], xmm14
    movaps [rsp + 8 * 10 + 16 * 9], xmm15

    ; TIB information
    ; stack low address (top of stack)
    mov r11, gs:[010h]
    mov [rsp + 8 * 10 + 16 * 10 + 8 * 0], r11

    ; stack high address (bottom of stack)
    mov r11, gs:[008h]
    mov [rsp + 8 * 10 + 16 * 10 + 8 * 1], r11

    ; deallocation stack address (top of stack - a few pages)
    mov r11, gs:[1478h]
    mov [rsp + 8 * 10 + 16 * 10 + 8 * 2 + 8 * 0], r11

    ; guaranteed stack bytes
    mov r11, gs:[1748h]
    mov [rsp + 8 * 10 + 16 * 10 + 8 * 2 + 8 * 1], r11

    ; current context is now fully saved
    ; switch to new context

    ; restore TIB information
    ; stack low address (top of stack)
    mov r11, [rcx + 8 * 10 + 16 * 10 + 8 * 0]
    mov gs:[010h], r11

    ; stack high address (bottom of stack)
    mov r11, [rcx + 8 * 10 + 16 * 10 + 8 * 1]
    mov gs:[008h], r11

    ; deallocation stack address (top of stack - a few pages)
    mov r11, [rcx + 8 * 10 + 16 * 10 + 8 * 2 + 8 * 0]
    mov gs:[1478h], r11

    ; guaranteed stack bytes
    mov r11, [rcx + 8 * 10 + 16 * 10 + 8 * 2 + 8 * 1]
    mov gs:[1748h], r11

    mov rsp, [rcx + 8 * 1] ; rsp
    ; now stack is setup

    ; switch to context in RDX
    ; restore non volatile register
    mov r12, [rsp + 8 * 2]
    mov r13, [rsp + 8 * 3]
    mov r14, [rsp + 8 * 4]
    mov r15, [rsp + 8 * 5]
    mov rdi, [rsp + 8 * 6]
    mov rsi, [rsp + 8 * 7]
    mov rbx, [rsp + 8 * 8]
    mov rbp, [rsp + 8 * 9]

    movaps xmm6,  [rsp + 8 * 10 + 16 * 0]
    movaps xmm7,  [rsp + 8 * 10 + 16 * 1]
    movaps xmm8,  [rsp + 8 * 10 + 16 * 2]
    movaps xmm9,  [rsp + 8 * 10 + 16 * 3]
    movaps xmm10, [rsp + 8 * 10 + 16 * 4]
    movaps xmm11, [rsp + 8 * 10 + 16 * 5]
    movaps xmm12, [rsp + 8 * 10 + 16 * 6]
    movaps xmm13, [rsp + 8 * 10 + 16 * 7]
    movaps xmm14, [rsp + 8 * 10 + 16 * 8]
    movaps xmm15, [rsp + 8 * 10 + 16 * 9]

    add rsp, 272 ; pop WinContext

    ; execute on top function
    ; rcx = previous context
    ; rdx = user data
    mov r11, [rcx+ 0 * 0] ; rip

    ; setup for a return instruction inside the swapped-in context
    push r11
    mov rcx, r10 ; previous context
    ; rdx= pointer to user data
    jmp r8 ; call ontop function


swap_context_on_top ENDP
END