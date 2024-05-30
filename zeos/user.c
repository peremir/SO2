#include <libc.h>

char buff[24];

int pid;
int mutex = 0;

int addAsm(int par1, int par2);
int write(int fd, char * buffer, int size);
unsigned int gettime();
int getpid();


void print(char *buffer)
{
  write(1, buffer, strlen(buffer));
}

void printNum(int num)
{
  char *numBuff = "\0\0\0\0\0\0";
  itoa(num, numBuff);
  print(numBuff);
}

void func(int i)
{
  print("\nHOLA SOY UN THREAD");
  
  exit_thread();
}

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

    char c[4];
    int err = read(c, 2);
    
    char *buff = "\0\0\0\0\0\0";
    itoa(err, buff);
    print(buff);
    
    print(c);
    print("\n");

  /* Funcio addAsm suma en assembly */
  //int x = addAsm(0x42, 0x666);  
  create_thread((void*)func, 0);
 
  //Test de la syscall fork feta amb sysenter 
  
  
  int pid = fork();
  if (pid == 0)
  { 
    block();
    
    char *bufferC = "\0\0\0\0\0";
    print("\nCHILD Getpid: ");
    itoa(getpid(), bufferC);
    print(bufferC);
  }
  else if(pid > 0)
  {  
    char *bufferP = "\0\0\0\0\0";
    print("\nPARENT Getpid: ");
    itoa(getpid(), bufferP);
    print(bufferP);

    char *buff = "\0\0\0\0\0\0";
    read(buff, 4);
    print("\n"); print(buff);

    unblock(pid);  
  }
  
  /* Funcio que provoca un page fault exception */
  //pf(); 

  while(1) { }
}
