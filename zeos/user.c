
char buff[24];

int pid;
int mutex = 5;
int counter = 0;

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

void printl()
{
 print("\n");
}

void func(int id)
{
         
   for (int i = 0; i < 5; i++) {
       mutex_lock(&mutex); 
        // Sección crítica
        int temp = counter;
        temp++;
        counter = temp;
        
        print("\nThread ");
        printNum(id);
        print(" incremented counter to ");
        printNum(counter);
 mutex_unlock(&mutex);       
            }


    exit_thread();
}


int __attribute__ ((__section__(".text.main")))
  main(void)
{
  /* Next line, tries to move value 0 to CR3 register. */ 
  /* This register is a privileged one, and so it will raise an exception */
  
  /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

  mutex_init(&mutex);

    for (int i = 0; i < 3; i++) {
        create_thread((void*)func, i);
    }

/* 
El Output hauria de ser aixi, primer thread 0, dps 1, dps 2
 
Thread 0 incremented counter to 1
Thread 0 incremented counter to 2
Thread 0 incremented counter to 3
Thread 0 incremented counter to 4
Thread 0 incremented counter to 5
Thread 1 incremented counter to 6
Thread 1 incremented counter to 7
Thread 1 incremented counter to 8
Thread 1 incremented counter to 9
Thread 1 incremented counter to 10
Thread 2 incremented counter to 11
Thread 2 incremented counter to 12
Thread 2 incremented counter to 13
Thread 2 incremented counter to 14
Thread 2 incremented counter to 15
*/

  while(1) { }
}
