#include <asm.h>
ENTRY(addAsm)
 
push   %ebp
mov    %esp,%ebp
mov    0x8(%ebp),%edx
mov    0xc(%ebp),%eax
add    %edx,%eax
pop    %ebp
ret 

ENTRY (write) ;/* WriteWrapper */

push %ebp
mov %esp,%ebp
push %edx
mov 16(%ebp), %edx 
mov 12(%ebp), %ecx
mov 8(%ebp), %ebx
mov $4, %eax
int $0x80
pop %edx
pop %ebp
ret
