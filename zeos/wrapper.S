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
	pop %ecx
	pop %ecx
	ret

/* System write sysenter wrapper */ 
ENTRY(write)
	push   %ebp
	mov    %esp, %ebp
	push   %ebx
	mov    8(%ebp), %edx		// 1) Pas de Parámetres
	mov    12(%ebp), %ecx
	mov    16(%ebp), %ebx
	mov    $4, %eax			// 2) Dir quin servei ès
	call syscall_sysenter			// 3) INT
	cmpl   $0, %eax
	jge    write_fin        
	//neg    %eax
	movl   %eax, errno		// 4) Tractament d'errors - Save errno to eax
	mov    $-1, %eax
write_fin:
	pop    %ebx
	pop    %ebp
	ret 

/* int gettime() */
ENTRY(gettime)
	pushl %ebp
	movl %esp, %ebp
	movl $10, %eax
	call syscall_sysenter
	popl %ebp
	ret

        /* int getpid() */
ENTRY(getpid)
	pushl %ebp
	movl %esp, %ebp
	movl $20, %eax
	call syscall_sysenter
	popl %ebp
	ret

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
	
/* void exit() */
ENTRY(exit)
	pushl %ebp
	movl %esp, %ebp
	movl $1, %eax
	call syscall_sysenter
	popl %ebp
	ret

/* void block() */
ENTRY(block)
	pushl %ebp
	movl %esp, %ebp
	movl $25, %eax
	call syscall_sysenter
	popl %ebp
	ret

/* System unblock interrupt wrapper */ 
ENTRY(unblock)
	push   %ebp
	mov    %esp, %ebp
	push   %ebx
	mov    8(%ebp), %edx		// 1) Pas de Parámetres
	mov    $26, %eax			// 2) Dir quin servei ès
	call syscall_sysenter			// 3) INT
	cmpl   $0, %eax
	jge    unblock_fin        
	movl   %eax, errno		// 4) Tractament d'errors - Save errno to eax
	mov    $-1, %eax
unblock_fin:
	pop    %ebx
	pop    %ebp
	ret 

/* System read interrupt wrapper */ 
ENTRY(read)
	push   %ebp
	mov    %esp, %ebp
	push   %ebx
	mov    8(%ebp), %edx		// 1) Pas de Parámetres
	mov    12(%ebp), %ecx
	mov    $11, %eax			// 2) Dir quin servei ès
	call syscall_sysenter
	cmpl   $0, %eax
	jge    rd_fin        
	movl   %eax, errno		// 4) Tractament d'errors - Save errno to eax
	mov    $-1, %eax
rd_fin:
	pop    %ebx
	pop    %ebp
	ret

ENTRY (create_thread)
	push   %ebp
	mov    %esp, %ebp
	mov    8(%ebp), %edx		// 1) Pas de Parámetres
	mov    12(%ebp), %ecx
	mov    $12, %eax			// 2) Dir quin servei ès
	call syscall_sysenter			// 3) INT
	popl %ebp
	ret	

ENTRY (exit_thread)
    	pushl  %ebp
    	movl   %esp, %ebp
    	movl   $13, %eax 
    	call syscall_sysenter               
    	popl   %ebp
	ret 

ENTRY (mutex_init)
    pushl %ebp
    mov %esp, %ebp
    mov 8(%ebp), %edx       // first argument to ebx
    mov $5, %eax
    call syscall_sysenter
    popl %ebp
    ret

ENTRY (mutex_lock)
    pushl %ebp
    mov %esp, %ebp
    mov 8(%ebp), %edx       // first argument to ebx
    mov $6, %eax
    call syscall_sysenter
    popl %ebp
    ret

ENTRY (mutex_unlock)
    pushl %ebp
    mov %esp, %ebp
    mov 8(%ebp), %edx       // first argument to ebx
    mov $7, %eax
    call syscall_sysenter
    popl %ebp
    ret

ENTRY (dyn_mem)
    pushl %ebp
    mov %esp, %ebp
    mov 8(%ebp), %edx       // first argument to ebx
    mov $14, %eax
    call syscall_sysenter
    popl %ebp
    ret
