# 0 "wrapper.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
# 1 "wrapper.S"




# 1 "include/asm.h" 1
# 6 "wrapper.S" 2

.globl addAsm; .type addAsm, @function; .align 0; addAsm:
 push %ebp
 mov %esp,%ebp
 mov 0x8(%ebp),%edx
 mov 0xc(%ebp),%eax
 add %edx,%eax
 pop %ebp
 ret


nok:
 neg %eax
 mov %eax, errno
 mov $-1, %eax
 popl %ebp
 ret

.globl syscall_sysenter; .type syscall_sysenter, @function; .align 0; syscall_sysenter:
 push %ecx
 push %edx
 push $SYSENTER_RETURN
 push %ebp
 mov %esp, %ebp
 sysenter
.globl SYSENTER_RETURN; .type SYSENTER_RETURN, @function; .align 0; SYSENTER_RETURN:
 pop %ebp
 pop %edx
 pop %ecx
 pop %ecx
 ret


.globl write; .type write, @function; .align 0; write:
 push %ebp
 mov %esp, %ebp
 push %ebx
 mov 8(%ebp), %edx
 mov 12(%ebp), %ecx
 mov 16(%ebp), %ebx
 mov $4, %eax
 call syscall_sysenter
 cmpl $0, %eax
 jge write_fin

 movl %eax, errno
 mov $-1, %eax
write_fin:
 pop %ebx
 pop %ebp
 ret


.globl gettime; .type gettime, @function; .align 0; gettime:
 pushl %ebp
 movl %esp, %ebp
 movl $10, %eax
 call syscall_sysenter
 popl %ebp
 ret


.globl getpid; .type getpid, @function; .align 0; getpid:
 pushl %ebp
 movl %esp, %ebp
 movl $20, %eax
 call syscall_sysenter
 popl %ebp
 ret


.globl fork; .type fork, @function; .align 0; fork:
 pushl %ebp
 movl %esp, %ebp
 movl $2, %eax
 call syscall_sysenter
 test %eax, %eax
 js nok
 popl %ebp
 ret


.globl exit; .type exit, @function; .align 0; exit:
 pushl %ebp
 movl %esp, %ebp
 movl $1, %eax
 call syscall_sysenter
 popl %ebp
 ret


.globl block; .type block, @function; .align 0; block:
 pushl %ebp
 movl %esp, %ebp
 movl $25, %eax
 call syscall_sysenter
 popl %ebp
 ret


.globl unblock; .type unblock, @function; .align 0; unblock:
 push %ebp
 mov %esp, %ebp
 push %ebx
 mov 8(%ebp), %edx
 mov $26, %eax
 call syscall_sysenter
 cmpl $0, %eax
 jge unblock_fin
 movl %eax, errno
 mov $-1, %eax
unblock_fin:
 pop %ebx
 pop %ebp
 ret


.globl read; .type read, @function; .align 0; read:
 push %ebp
 mov %esp, %ebp
 push %ebx
 mov 8(%ebp), %edx
 mov 12(%ebp), %ecx
 mov $11, %eax
 call syscall_sysenter
 cmpl $0, %eax
 jge rd_fin
 movl %eax, errno
 mov $-1, %eax
rd_fin:
 pop %ebx
 pop %ebp
 ret

.globl create_thread; .type create_thread, @function; .align 0; create_thread:
 push %ebp
 mov %esp, %ebp
 mov 8(%ebp), %edx
 mov 12(%ebp), %ecx
 mov $12, %eax
 call syscall_sysenter
 popl %ebp
 ret

.globl exit_thread; .type exit_thread, @function; .align 0; exit_thread:
     pushl %ebp
     movl %esp, %ebp
     movl $13, %eax
     call syscall_sysenter
     popl %ebp
 ret

.globl mutex_init; .type mutex_init, @function; .align 0; mutex_init:
    pushl %ebp
    mov %esp, %ebp
    mov 8(%ebp), %edx
    mov $5, %eax
    call syscall_sysenter
    popl %ebp
    ret

.globl mutex_lock; .type mutex_lock, @function; .align 0; mutex_lock:
    pushl %ebp
    mov %esp, %ebp
    mov 8(%ebp), %edx
    mov $6, %eax
    call syscall_sysenter
    popl %ebp
    ret

.globl mutex_unlock; .type mutex_unlock, @function; .align 0; mutex_unlock:
    pushl %ebp
    mov %esp, %ebp
    mov 8(%ebp), %edx
    mov $7, %eax
    call syscall_sysenter
    popl %ebp
    ret

.globl dyn_mem; .type dyn_mem, @function; .align 0; dyn_mem:
    pushl %ebp
    mov %esp, %ebp
    mov 8(%ebp), %edx
    mov $14, %eax
    call syscall_sysenter
    popl %ebp
    ret
