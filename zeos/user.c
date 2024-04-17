#include <libc.h>

char buff[24];

int pid;

int addAsm(int par1, int par2);
int write(int fd, char * buffer, int size);
unsigned int gettime();
int getpid();

void pf()
{
  char *p = 0;
  *p = 'x';
}

int __attribute__ ((__section__(".text.main")))
  main(void)
{
  /* Next line, tries to move value 0 to CR3 register. */ 
  /* This register is a privileged one, and so it will raise an exception */
  
  /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */



  /* Funcio addAsm suma en assembly */
  //int x = addAsm(0x42, 0x666);  
  
  /* Crida syscall write per escriure per pantalla com a usuari */
  if(write(1,"\nsyscall write funcionant :)",strlen("\nsyscall write funcionant :)")) < 0) perror();
  
  //Test de la syscall gettime feta amb sysenter
  char *buffer = "\0\0\0\0\0";
  write(1, "\nGettime 1: ", 12);
  itoa(gettime(), buffer);
  write(1, buffer, 6);
  
  //Test de la syscall getpid feta amb sysenter 
  char *buffer2 = "\0\0\0\0\0";
  write(1, "\nGetpid: ", 9); 
  itoa(getpid(), buffer2);
  write(1, buffer2, 6);
  
  //Test de la syscall fork feta amb sysenter
  int pid = fork();
  if (pid == 0)
  {
    char *bufferC = "\0\0\0\0\0";
    write(1, "\nCHILD Getpid: ", 15);
    itoa(getpid(), bufferC);
    write(1, bufferC, 6);
  }
  else if(pid > 0)
  {
    char *bufferP = "\0\0\0\0\0";
    write(1, "\nPARENT Getpid: ", 16);
    itoa(getpid(), bufferP);
    write(1, bufferP, 6);
  }
  else
  {
    write(1, "\nERROR", 6);
  }

  /* Funcio que provoca un page fault exception */
  //pf(); 

  while(1) { }
}
