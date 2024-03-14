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

ENTRY (gettime)
	push   %ebp
	mov    %esp, %ebp
	mov    $10, %eax		// 2) Dir quin servei és
	int    $0x80			// 3) INT
	cmpl   $0, %eax
	jge    gettime_fin
	//neg    %eax
	movl   %eax, errno		// 4) Tractament d'errors - Save errno to eax
	mov    $-1, %eax
gettime_fin:
	pop    %ebp 
	ret



/*
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
*/

