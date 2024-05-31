#include"utils_joc.h"

char buff[24];

int pid;
int mutex = 0;

int counter = 0;
int zaza = 0;

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

void func(int id)
{
 
   mutex_lock(&mutex); 
   for (int i = 0; i < 5; i++) {
              // Sección crítica
        //while (gettime()<(500*(5-id))){}
        for(int j = 0; j < 1000000; j++) {
          zaza = 1+id; 
        }

        counter++;
        print("\nThread ");
        printNum(id);
        print(" incremented counter to ");
        printNum(counter);
        
   }
     
    print("\n zaza: ");
    printNum(zaza);
    print("\n");
	  mutex_unlock(&mutex);       

    exit_thread();
}


int __attribute__ ((__section__(".text.main")))
  main(void)
{
  
  g_fill_screen('E',RED,YELLOW);
  while(1) { }
}
