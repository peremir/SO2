
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

void printl()
{
 print("\n");
}

void func(int i)
{
  print("\nHOLA SOY UN THREAD");
  
  exit_thread();
}

int __attribute__ ((__section__(".text.main")))
  main(void)
{
  /* Next line, tries to move value 0 to CR3 register. */ 
  /* This register is a privileged one, and so it will raise an exception */
  
  /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
  
     // Alocatamos memoria
     char *buffer = dyn_mem(4096*2);
    
     // Escribimos en memoria
     for (int i = 0; i < 4096*2; ++i) 
     {
         buffer[i] = "a";
     }
     print("\nRecorremos la memoria por primera vez sin ningun problema");
    
     buffer[4096] = "a"; // No deberia dar page_fault
     print("\nA mi SI me deberias ver");

     ////printl(); print(buffer[4096]);
     
     // Hacemos free de parte de la memoria
     dyn_mem(-4096*2);
     
     //buffer[4096] = "a"; // Deberia dar page_fault
     //print("A mi no me deberias ver el pelo\n");
     
  /*
  create_thread((void*)func, 0);

   
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
    
    char b[5];
    read(b, 4);
    print("\n"); print(b);

    unblock(pid);
  }
  */
  
   while(1) { }
}
