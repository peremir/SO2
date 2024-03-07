#include <libc.h>

char buff[24];

int pid;

//definir funcio 
int addAsm(int par1, int par2);
int write(int fd, char * buffer, int size);
unsigned int gettime();


int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

  //int x = addAsm(0x42, 0x666);  
  

  if(write(1,"\nsyscall write funcionant :)",strlen("\nsyscall write funcionant :)")) < 0)
  {
    perror();
  }
  

  while(1) { }
}
