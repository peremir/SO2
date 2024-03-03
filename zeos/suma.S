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

push %ebp
mov %esp, %ebp
//1) Pas de Parámetres

mov 8(%ebp), %edx
mov 12(%ebp), %ecx
mov 16(%ebp), %ebx

//2) Dir quin servei és
mov $4, %eax

//3) INT
int $0x80

cmpl $0, %eax
jge fin

// save -eax to errno here. i dont know how to do that yet.
neg %eax
movl %eax, errno
mov $-1, %eax


fin:
pop %ebp
//4) Tractament d'errors
ret
