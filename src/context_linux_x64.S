.type get_context, @function
.type set_context, @function
.type swap_context, @function
.type swap_context_on_top, @function
.global get_context
.global set_context
.global swap_context
.global swap_context_on_top

# Store the current context inside a given LinuxContext* (stored in rdi, 1st arg)
# rdi = LinuxContext* pCurrentContext
get_context:
    # Store return address in RIP of Context object
    movq (%rsp), %r8
    movq %r8, 8*0(%rdi)

    # Store address of stack pointer, as it was before the call
    leaq 8(%rsp), %r8
    movq %r8, 8*1(%rdi)

    # Then store non-volatile registers
    movq %rbx, 8*2(%rdi)
    movq %rbp, 8*3(%rdi)
    movq %r12, 8*4(%rdi)
    movq %r13, 8*5(%rdi)
    movq %r14, 8*6(%rdi)
    movq %r15, 8*7(%rdi)

    # Contrary to Windows, no need to store xmm registers, not preserved in System-V ABI
    ret

# Restores the context from a LinuxContext* (stored in rdi).
# rdi = LinuxContext* pCurrentContext
set_context:
    movq 8*0(%rdi), %r11 # get address to resume execution from
    movq 8*1(%rdi), %rsp # restore stack

    # Restore non-volatile registers
    movq 8*2(%rdi), %rbx
    movq 8*3(%rdi), %rbp
    movq 8*4(%rdi), %r12
    movq 8*5(%rdi), %r13
    movq 8*6(%rdi), %r14
    movq 8*7(%rdi), %r15

    # Push return address to stack, so that RET jumps to it
    pushq %r11
    ret

# Resumes the context by calling the given function in RDX, with its first argument being what is in RSI
#  When returning, that function will return to the context inside RDI
# RDI = LinuxContext* context to switch to
# RSI  = Pointer to user data
# RDX  = Address of function to resume on.
#       This function will get the pointer to the previous context (the one calling swap_context_on_top) and the pointer to user data (in this order)
swap_context_on_top:
    popq %r11 # read return address

    # copy origin context to stack
    subq $64, %rsp
    movq %rsp, %r9 # pointer to saved context

    movq %r11, 8*0(%rsp)

    # (same as get_context but with rsp instead of rdi)
    # Store address of stack pointer, as it was before the call
    movq %rsp, 8*1(%rsp)

    # Then store non-volatile registers
    movq %rbx, 8*2(%rsp)
    movq %rbp, 8*3(%rsp)
    movq %r12, 8*4(%rsp)
    movq %r13, 8*5(%rsp)
    movq %r14, 8*6(%rsp)
    movq %r15, 8*7(%rsp)

    # Context is ready, switch to new one

    movq 8*0(%rdi), %r11 # get address to resume execution from
    movq 8*1(%rdi), %rsp # restore stack

    # Restore non-volatile registers
    movq 8*2(%rdi), %rbx
    movq 8*3(%rdi), %rbp
    movq 8*4(%rdi), %r12
    movq 8*5(%rdi), %r13
    movq 8*6(%rdi), %r14
    movq 8*7(%rdi), %r15

    addq $64, %rsp

    # execute on top function
    # rdi = previous context
    # rsi = user data not touched by this routine
    pushq %r11
    movq %r9, %rdi
    jmp *%rdx
