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
int quantum_left;


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

int ret_from_fork() 
{
  return 0;
}

int sys_fork()
{
  //int PID=-1;

  // creates the child process

  //Comprobamos que la freequeue esté libre para asignar un proceso
  if (list_empty(&freequeue)) 
  {
    return -ENOMEM;
  }
  
  //obtenemos de la freequeue una pcb libre
  struct list_head *free = list_first(&freequeue);
  list_del(free);
  union task_union *pcb = (union task_union*)list_head_to_task_struct(free);
  
  //copiamos el task_union del padre al hijo
  copy_data(current(),pcb,sizeof(union task_union));
  
  //como se ha copiado todo exactamente igual del padre al hijo, 
  //hay que asignarle otro directorio de páginas lógicas
  allocate_DIR(&(pcb->task));
  
  /*
  pag_i < 256
  set_ss_pag(hijo, pag_i, get_frame(padre,pag_i))
  */
  page_table_entry *PT_child = get_PT(&(pcb->task));
  page_table_entry *PT_parent = get_PT(current());

  int pag;
  int frame;
  for (pag = PAG_LOG_INIT_DATA; pag < PAG_LOG_INIT_DATA + NUM_PAG_DATA; pag++) 
  {
    frame = alloc_frame();
    if (frame != -1) 
    {
      set_ss_pag(PT_child, pag, frame);
    }
    else 
    {
      for (int i = PAG_LOG_INIT_DATA; i < pag; i++) 
      {
        free_frame(get_frame(PT_child, i));
				del_ss_pag(PT_child, PAG_LOG_INIT_DATA+i);
      }
      list_add_tail(free, &freequeue);
      return -ENOMEM;
    }
  }

  for (pag = 0; pag < NUM_PAG_KERNEL; pag++) 
  {
    set_ss_pag(PT_child, pag, get_frame(PT_parent, pag));
  }
  for (pag = PAG_LOG_INIT_CODE; pag < PAG_LOG_INIT_CODE + NUM_PAG_CODE; pag++) 
  {
    set_ss_pag(PT_child, pag, get_frame(PT_parent, pag));
  }

  int NUM_PAG_USER = NUM_PAG_DATA + NUM_PAG_CODE;
  for (pag = PAG_LOG_INIT_DATA; pag < PAG_LOG_INIT_DATA + NUM_PAG_DATA; pag++) 
  {
    set_ss_pag(PT_parent, pag + NUM_PAG_USER, get_frame(PT_child, pag));
    copy_data((void *)(pag << 12), (void *)((pag+NUM_PAG_USER) << 12), PAGE_SIZE);
    del_ss_pag(PT_parent, pag+NUM_PAG_USER);
  }
  
  set_cr3(get_DIR(current()));
  
  pcb->task.kernel_esp = &(pcb->stack[KERNEL_STACK_SIZE-19]); // 17-2 por el @ret_from_fork y el ebp.

  pcb->stack[KERNEL_STACK_SIZE-19] = 0;
  pcb->stack[KERNEL_STACK_SIZE-18] = (DWord)ret_from_fork;

  pcb->task.PID = pids;
  pids++;

  INIT_LIST_HEAD(&(pcb->task.child_list));
 	//(union task_union*)list_head_to_task_struct(current()->child_list);
	
  list_add_tail(&pcb->task.bro, &(current()->child_list));

  list_add_tail(free, &readyqueue);
  return pcb->task.PID;
}

void sys_exit()
{
  struct task_struct *pcb = current();
  struct task_struct *pcb_parent = pcb->parent;	

  struct list_head* it;
  list_for_each(it, &pcb_parent->child_list)
	{
		struct task_struct *pcb_child = list_head_to_task_struct(it);	
		pcb_child->parent = idle_task; 
 		list_add_tail(it, &(idle_task->child_list));
	}

  free_user_pages(current());
  update_process_state_rr(current(), &freequeue);
  sched_next_rr();
}

void sys_block()
{
  if (pending_unblocks > 0) 
  {
    pending_unblocks--;	
  }
  else 
  {	
    struct list_head* list = &current()->list;
    list_del(list);
	list_add_tail(list, &blocked);
    sched_next_rr();
    }
}




int sys_unblock(int pid)
{
    struct list_head* it;
    list_for_each(it, &(current()->child_list))
	{
        struct task_struct *pcb_child = list_head_to_task_struct(it);	
	    char * buffer = "\0\0\0\0\0";
        itoa(pcb_child->PID, buffer);
        printk(buffer);

        if (pcb_child->PID == pid) {
            if (list_first(&(pcb_child->list)) == list_first(&blocked)) {
                //desbloquearlo
                struct list_head * l = &(pcb_child->list);
                list_del(l);
                list_add_tail(l, &readyqueue);
                return 0;
            }
            else {
                pending_unblocks++;
                return 0;
            }
        }
    }
    //ESTO TIENE QUE SER -1 !!!!
    return 33;
}
/*
 * 
 *
 :	 * if ((pid is current()->child) && process_pid is blocked) {
	 *	desbloquejar (readyqueue nosek);
	 *	return 0;
	 * }
	 * if (process_pid is NOT blocked) {
	 *	pending_unblocks++;
	 * 	return 0;
	 * }
	 * 
	 * return -1; 
	*/	



