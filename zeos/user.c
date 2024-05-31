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

  char *fufu = "\[20;20fhola";
  write(1, fufu, strlen(fufu));
  char *ff = "\[1mpenis";

  write(1, ff, strlen(ff));

//    print("\[31,43mhola");
    while(1) { }
}
