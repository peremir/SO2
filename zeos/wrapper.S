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
	pop %edx
	pop %ecx
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

/* int write(int fd, char *buffer, int size) 
ENTRY(write)
	pushl %ebp
	movl %esp, %ebp
	pushl %ebx;  // Save EBX, ESI and EDI if modified
	movl $4, %eax
	movl 0x8(%ebp), %ebx;	//fd
	movl 0xC(%ebp), %ecx;	//buffer
	movl 0x10(%ebp), %edx;	//size
	call syscall_sysenter
	popl %ebx
	test %eax, %eax
	js nok	// if (eax < 0) -> 
	popl %ebp
	ret */

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
	int    $0x80			// 3) INT
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
	int    $0x80			// 3) INT
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
	push   %ebx
	mov    8(%ebp), %edx		// 1) Pas de Parámetres
	mov    12(%ebp), %ecx
	mov    $12, %eax			// 2) Dir quin servei ès
	int    $0x80			// 3) INT
	cmpl   $0, %eax
	jge    ct_fin        
	movl   %eax, errno		// 4) Tractament d'errors - Save errno to eax
	mov    $-1, %eax
ct_fin:
	pop    %ebx
	pop    %ebp
	ret



ENTRY (exit_thread)
    pushl %ebp
    movl %esp, %ebp

    movl $13, %eax // id de la syscall exit() -> 1

    pushl $ext_thr_return_user // @ a la que sysenter debe devolver
    pushl %ebp              // el ebp actual, que debera usarse al volver
    movl %esp, %ebp

    sysenter                // saltamos a la primera entrada de la sys_call_table

    // Exit no devuelve error. Es más, nunca devuelve por donde
    // se la ha llamado. Por lo tanto, las siguientes lineas de
    // codigo nunca se ejecutan.

ext_thr_return_user:
    popl %ebp
    add $4, %esp // pop of return address.

    leave
    ret 
