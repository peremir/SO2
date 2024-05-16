/*
 * wrapper.S - Wrapper functions code
 */

#include <asm.h>

ENTRY(addAsm)
	push   %ebp
	mov    %esp,%ebp
	mov    0x8(%ebp),%edx
	mov    0xc(%ebp),%eax
	add    %edx,%eax
	pop    %ebp
	ret

/* System write interrupt wrapper */
ENTRY(write)
	push   %ebp
	mov    %esp, %ebp
	push   %ebx
	mov    8(%ebp), %edx		// 1) Pas de Parámetres
	mov    12(%ebp), %ecx
	mov    16(%ebp), %ebx
	mov    $4, %eax			// 2) Dir quin servei ès
	int    $0x80			// 3) INT
	cmpl   $0, %eax
	jge    write_fin        
	//neg    %eax
	movl   %eax, errno		// 4) Tractament d'errors - Save errno to eax
	mov    $-1, %eax
write_fin:
	pop    %ebx
	pop    %ebp
	ret

/* System gettime sysenter wrapper */
ENTRY(gettime)
	pushl %ebp
	movl %esp, %ebp
	movl $10, %eax
	pushl %ecx
	pushl %edx
	pushl $return_gettime
	pushl %ebp
	movl %esp, %ebp
	sysenter
return_gettime:
	popl %ebp
	addl $4, %esp
	popl %edx
	popl %ecx
	cmpl $0, %eax
        jge gettime_fin
        //negl %eax
        movl %eax, errno
        movl $-1, %eax
gettime_fin:
	popl %ebp
	ret

/* System getpid sysenter wrapper 
ENTRY(getpid)
        pushl %ebp
        movl %esp, %ebp
        movl $20, %eax
        pushl %ecx
        pushl %edx
        pushl $return_getpid
        pushl %ebp
        movl %esp, %ebp
        sysenter
return_getpid:
        popl %ebp
        addl $4, %esp
        popl %edx
        popl %ecx
        cmpl $0, %eax
        jge getpid_fin
        //negl %eax
        movl %eax, errno
        movl $-1, %eax
getpid_fin:
        popl %ebp
        ret*/
        
        /* int getpid() */
ENTRY(getpid)
	pushl %ebp
	movl %esp, %ebp
	movl $20, %eax
	call syscall_sysenter
	popl %ebp
	ret

/* System fork sysenter wrapper 
ENTRY(fork)
        pushl %ebp
        movl %esp, %ebp 
        movl $2, %eax
        pushl %ecx
        pushl %edx
        pushl $return_fork
        pushl %ebp
        movl %esp, %ebp
        sysenter
return_fork:
        popl %ebp
        addl $4, %esp
        popl %edx
        popl %ecx
        cmpl $0, %eax
        jge fork_fin
        //negl %eax             
        movl %eax, errno        
        movl $-1, %eax  
fork_fin:             
        popl %ebp       
        ret/*
        
/* int fork() */
ENTRY(fork)
	pushl %ebp
	movl %esp, %ebp
	movl $2, %eax
	call syscall_sysenter
	test %eax, %eax
	js nok	// if (eax < 0) -->
	popl %ebp
	ret
	
/* Common code for negative return */
nok:
	neg %eax
	mov %eax, errno
	mov $-1, %eax
	popl %ebp
	ret
	
ENTRY(syscall_sysenter)
	push %ecx
	push %edx
	push $SYSENTER_RETURN
	push %ebp
	mov %esp, %ebp
	sysenter
ENTRY(SYSENTER_RETURN)
	pop %ebp
	pop %edx
	pop %edx
	pop %ecx
	ret

/* System exit sysenter wrapper */
ENTRY(exit)
	pushl %ebp
    	movl %esp, %ebp
    	movl $1, %eax 		// id de la syscall exit() -> 1
    	pushl $exit_return_user // @ a la que sysenter debe devolver
   	pushl %ebp              // el ebp actual, que debera usarse al volver
    	movl %esp, %ebp
    	sysenter                // saltamos a la primera entrada de la sys_call_table
exit_return_user:
   	popl %ebp
    	add $4, %esp // pop of return address.
    	leave
    	ret

/* System block sysenter wrapper 
ENTRY(block)
        pushl %ebp
        movl %esp, %ebp
        movl $25, %eax 
        pushl $block_return_user // @ a la que sysenter debe devolver
        pushl %ebp              // el ebp actual, que debera usarse al volver
        movl %esp, %ebp
        sysenter                // saltamos a la primera entrada de la sys_call_table
block_return_user:
        popl %ebp
        add $4, %esp // pop of return address.
        leave
        ret

/* System unblock sysenter wrapper 
ENTRY(unblock)
        pushl %ebp
        movl %esp, %ebp
        movl $26, %eax
        movl 8(%ebp), %edx 
        pushl $unblock_return_user // @ a la que sysenter debe devolver
        pushl %ebp              // el ebp actual, que debera usarse al volver
        movl %esp, %ebp
        sysenter                // saltamos a la primera entrada de la sys_call_table
unblock_return_user:
        add $8, %esp // pop of return address.
        popl %ebp
        ret
*/

/* System block sysenter wrapper */
ENTRY(block)
        pushl %ebp
        movl %esp, %ebp
        movl $25, %eax  //; Replace SYS_BLOCK with the actual syscall number for block
        pushl %ecx
        pushl %edx
        pushl $return_block
        pushl %ebp
        movl %esp, %ebp
        sysenter
return_block:
        popl %ebp
        addl $4, %esp
        popl %edx
        popl %ecx
        cmpl $0, %eax
        jge block_fin
        //negl %eax
        movl %eax, errno
        movl $-1, %eax
block_fin:
        popl %ebp
        ret

/* System unblock sysenter wrapper */
ENTRY(unblock)
        pushl %ebp
        movl %esp, %ebp
        movl 8(%ebp), %ebx      // ; Move the pid argument into %ebx
        movl $26, %eax  //; Replace SYS_UNBLOCK with the actual syscall number for unblock
        pushl %ecx
        pushl %edx
        pushl $return_unblock
        pushl %ebp
        movl %esp, %ebp
        sysenter
return_unblock:
        popl %ebp
        addl $4, %esp
        popl %edx
        popl %ecx
        cmpl $0, %eax
        jge unblock_fin
        //negl %eax
        movl %eax, errno
        movl $-1, %eax
unblock_fin:
        popl %ebp
        ret

