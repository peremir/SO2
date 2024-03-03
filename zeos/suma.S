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
mov 8(%ebp), %edx 
mov 12(%ebp), %ecx
mov 16(%ebp), %ebx
mov $4, %eax
int $0x80
pop %ebp
ret
