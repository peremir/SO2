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

.globl write; .type write, @function; .align 0; write:
 push %ebp
 mov %esp, %ebp
 push %ebx
 mov 8(%ebp), %edx
 mov 12(%ebp), %ecx
 mov 16(%ebp), %ebx
 mov $4, %eax
 int $0x80
 cmpl $0, %eax
 jge write_fin

 movl %eax, errno
 mov $-1, %eax
write_fin:
 pop %ebx
 pop %ebp
 ret

.globl gettime; .type gettime, @function; .align 0; gettime:
 push %ebp
 mov %esp, %ebp
 mov $10, %eax
 int $0x80
 cmpl $0, %eax
 jge gettime_fin

 movl %eax, errno
 mov $-1, %eax
gettime_fin:
 pop %ebp
 ret



.globl getpid; .type getpid, @function; .align 0; getpid:
 push %ebp
 mov %esp, %ebp
 mov $20, %eax
 int $0x80
 cmpl $0, %eax
 jge getpid_fin
 movl %eax, errno
 mov $-1, %eax
getpid_fin:
 pop %ebp
 ret

.globl fork; .type fork, @function; .align 0; fork:
 push %ebp
 mov %esp, %ebp
 mov $2, %eax
 int $0x80
 cmpl $0, %eax
 jge getpid_fin
 movl %eax, errno
 mov $-1, %eax
fork_fin:
 pop %ebp
 ret
