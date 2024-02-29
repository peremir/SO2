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

//1) Pas de Parámetres
mov 16(%ebp), %edx
mov 12(%ebp), %ecx
mov 8(%ebp), %ebx

//2) Dir quin servei és
mov $4, %eax

//3) INT
int $0x80

//4) Tractament d'errors
ret

