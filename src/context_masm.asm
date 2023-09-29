PUBLIC get_context
PUBLIC set_context
PUBLIC swap_context
PUBLIC swap_context_on_top

.code
; Store the current context inside a given WinContext* (stored in rcx)
; RCX = WinContext* pCurrentContext
get_context PROC
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

    ret
get_context ENDP

; Restores the context from a WinContext* (stored in rcx). Can be used to return to a context made with get_context, or
;  a brand new context
; RCX = WinContext* pContextToSwitchTo
set_context PROC
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

    jmp r11 ; go to return address of get_context
set_context ENDP

; Swaps the current context (will be stored in [RCX]) with another context (read from [RDX])
; RCX = WinContext* storage for current context
; RDX = WinContext* context to switch to
swap_context PROC
    mov r11, [rsp] ; read return address
    mov [rcx + 8*0], r11 ; store return address (RIP) of previous frame

    mov r11, rsp
    add r11, 8
    mov [rcx + 8*1], r11 ; store stack pointer of previous frame

    ; store non volatile registers
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

    ; current context is now fully saved
    ; switch to context in RDX
    mov r11, [rdx + 0 * 0] ; rsi
    mov rsp, [rdx + 8 * 1] ; rsp

    ; restore non volatile register
    mov r12, [rdx + 8 * 2]
    mov r13, [rdx + 8 * 3]
    mov r14, [rdx + 8 * 4]
    mov r15, [rdx + 8 * 5]
    mov rdi, [rdx + 8 * 6]
    mov rsi, [rdx + 8 * 7]
    mov rbx, [rdx + 8 * 8]
    mov rbp, [rdx + 8 * 9]

    movaps xmm6,  [rdx + 8 * 10 + 16 * 0]
    movaps xmm7,  [rdx + 8 * 10 + 16 * 1]
    movaps xmm8,  [rdx + 8 * 10 + 16 * 2]
    movaps xmm9,  [rdx + 8 * 10 + 16 * 3]
    movaps xmm10, [rdx + 8 * 10 + 16 * 4]
    movaps xmm11, [rdx + 8 * 10 + 16 * 5]
    movaps xmm12, [rdx + 8 * 10 + 16 * 6]
    movaps xmm13, [rdx + 8 * 10 + 16 * 7]
    movaps xmm14, [rdx + 8 * 10 + 16 * 8]
    movaps xmm15, [rdx + 8 * 10 + 16 * 9]

    ; restore TIB information
    ; stack low address (top of stack)
    mov r10, [rdx + 8 * 10 + 16 * 10 + 8 * 0]
    mov gs:[010h], r10

    ; stack high address (bottom of stack)
    mov r10, [rdx + 8 * 10 + 16 * 10 + 8 * 1]
    mov gs:[008h], r10

    jmp r11 ; switch execution to target context
swap_context ENDP

; Slightly different version of swap_context,
;  resuming the context by calling the given function in R9, with its first argument being what is in R8
;  When returning, that function will return to the context inside RDX
; RCX = WinContext* current context
; RDX = WinContext* context to switch to
; R8  = Pointer to user data
; R9  = Address of function to resume on.
swap_context_on_top PROC
    mov r11, [rsp] ; read return address
    mov [rcx + 8*0], r11 ; store return address (RIP) of previous frame

    mov r11, rsp
    add r11, 8
    mov [rcx + 8*1], r11 ; store stack pointer of previous frame

    ; store non volatile registers
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

    ; current context is now fully saved
    ; switch to context in RDX
    mov r11, [rdx + 0 * 0] ; rsi
    mov rsp, [rdx + 8 * 1] ; rsp

    ; restore non volatile register
    mov r12, [rdx + 8 * 2]
    mov r13, [rdx + 8 * 3]
    mov r14, [rdx + 8 * 4]
    mov r15, [rdx + 8 * 5]
    mov rdi, [rdx + 8 * 6]
    mov rsi, [rdx + 8 * 7]
    mov rbx, [rdx + 8 * 8]
    mov rbp, [rdx + 8 * 9]

    movaps xmm6,  [rdx + 8 * 10 + 16 * 0]
    movaps xmm7,  [rdx + 8 * 10 + 16 * 1]
    movaps xmm8,  [rdx + 8 * 10 + 16 * 2]
    movaps xmm9,  [rdx + 8 * 10 + 16 * 3]
    movaps xmm10, [rdx + 8 * 10 + 16 * 4]
    movaps xmm11, [rdx + 8 * 10 + 16 * 5]
    movaps xmm12, [rdx + 8 * 10 + 16 * 6]
    movaps xmm13, [rdx + 8 * 10 + 16 * 7]
    movaps xmm14, [rdx + 8 * 10 + 16 * 8]
    movaps xmm15, [rdx + 8 * 10 + 16 * 9]

    ; restore TIB information
    ; stack low address (top of stack)
    mov r10, [rdx + 8 * 10 + 16 * 10 + 8 * 0]
    mov gs:[010h], r10

    ; stack high address (bottom of stack)
    mov r10, [rdx + 8 * 10 + 16 * 10 + 8 * 1]
    mov gs:[008h], r10

    mov rcx, r8 ; argument of function on top will be user data
    push r11
    jmp r9 ; switch execution to function on top
swap_context_on_top ENDP
END