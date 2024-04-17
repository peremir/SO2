/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include <errno.h>

#define LECTURA 0
#define ESCRIPTURA 1


extern unsigned int zeos_ticks;

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -EBADF; /* Bad file number -9 */
  if (permissions!=ESCRIPTURA) return -EACCES; /* Permission denied -13 */
  
  return 0; /* No error */
}

int sys_write(int fd, char * buffer, int size) 
{        
  int error = check_fd(fd, ESCRIPTURA);
	
  if (error < 0) return error; /* Check the check_fd function returns */

  if (buffer == NULL) return -EFAULT; /* Bad address -14 */

  if (size <= 0) return -EINVAL; /* Invalid argument -22 */
  
  // Print char by char so the stacj does not fill
  for (int i = 0; i < size; ++i)
  {
    // Initialize the char that we are going to write
    char buff;

    // Copy from user one char from the buffer moved by i
    error = copy_from_user(buffer + i, &buff, sizeof(buff));
    if (error < 0) return error;

    // Write the 1 char coppied
    sys_write_console(&buff, sizeof(buff));
  }

  return 0; /* No error */
}

unsigned long sys_gettime() 
{
  return zeos_ticks;
}

int sys_ni_syscall()
{
  return -ENOSYS; /* Invalid system call number -38 */
}

int sys_getpid()
{
  return current()->PID;
}

int ret_from_fork() {
    return 0;
}

int sys_fork()
{
  //int PID=-1;

  // creates the child process
  if (list_empty(&freequeue))
  {
    return -ENOMEM; /* Out of memory */
  }

  struct list_head *free = list_first(&freequeue);
  list_del(free);
  union task_union *pcb = (union task_union*)list_head_to_task_struct(free);
  
  copy_data(current(), pcb, sizeof(union task_union));
  allocate_DIR((struct task_struct*)pcb); 
  //init stats
  
  ((struct task_struct*)pcb)->kernel_esp = &(pcb->stack[KERNEL_STACK_SIZE-19]);
  pcb->stack[KERNEL_STACK_SIZE-19] = 0;
  pcb->stack[KERNEL_STACK_SIZE-18] = (DWord)ret_from_fork;

  if ((copy_and_allocate_pages(current(), pcb)) < 0) 
  {
    list_add_tail(free, &freequeue);
    return -ENOMEM;
  }

  list_add_tail(free, &readyqueue);

  ((struct task_struct*)pcb)->PID = ++pids;
  
  task_switch(pcb);

  return ((struct task_struct*)pcb)->PID;
}

void sys_exit()
{
	
}


