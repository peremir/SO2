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
  struct list_head *lhcurrent = NULL;
  union task_union *uchild;
  
  /* Any free task_struct? */
  if (list_empty(&freequeue)) return -ENOMEM;

  lhcurrent=list_first(&freequeue);
  
  list_del(lhcurrent);

  struct task_struct *schild = list_head_to_task_struct(lhcurrent);

  uchild=(union task_union*)list_head_to_task_struct(lhcurrent);
  
  /* Copy the parent's task struct to child's */
  copy_data(current(), uchild, sizeof(union task_union));
  
  /* new pages dir */
  allocate_DIR((struct task_struct*)uchild);
  
  /* Allocate pages for DATA+STACK */
  int new_ph_pag, pag, i;
  page_table_entry *process_PT = get_PT(&uchild->task);
  for (pag=0; pag<NUM_PAG_DATA; pag++)
  {
    new_ph_pag=alloc_frame();
    if (new_ph_pag!=-1) /* One page allocated */
    {
      set_ss_pag(process_PT, PAG_LOG_INIT_DATA+pag, new_ph_pag);
    }
    else /* No more free pages left. Deallocate everything */
    {
      /* Deallocate allocated pages. Up to pag. */
      for (i=0; i<pag; i++)
      {
        free_frame(get_frame(process_PT, PAG_LOG_INIT_DATA+i));
        del_ss_pag(process_PT, PAG_LOG_INIT_DATA+i);
      }
      /* Deallocate task_struct */
      list_add_tail(lhcurrent, &freequeue);
      
      /* Return error */
      return -EAGAIN; 
    }
  }

  /* Copy parent's SYSTEM and CODE to child. */
  page_table_entry *parent_PT = get_PT(current());
  for (pag=0; pag<NUM_PAG_KERNEL; pag++)
  {
    set_ss_pag(process_PT, pag, get_frame(parent_PT, pag));
  }
  for (pag=0; pag<NUM_PAG_CODE; pag++)
  {
    set_ss_pag(process_PT, PAG_LOG_INIT_CODE+pag, get_frame(parent_PT, PAG_LOG_INIT_CODE+pag));
  }
  /* Copy parent's DATA to child. We will use TOTAL_PAGES-1 as a temp logical page to map to */
  for (pag=NUM_PAG_KERNEL; pag<NUM_PAG_KERNEL+NUM_PAG_DATA; pag++)
  {
    /* Map one child page to parent's address space. */
    set_ss_pag(parent_PT, pag+NUM_PAG_DATA+NUM_PAG_CODE, get_frame(process_PT, pag));
    copy_data((void*)(pag<<12), (void*)((pag+NUM_PAG_DATA+NUM_PAG_CODE)<<12), PAGE_SIZE);
    del_ss_pag(parent_PT, pag+NUM_PAG_DATA+NUM_PAG_CODE);
  }
  /* Deny access to the child's memory space */
  set_cr3(get_DIR(current()));
  
  int register_ebp;		/* frame pointer */
  /* Map Parent's ebp to child's stack */
  register_ebp = (int) get_ebp();
  register_ebp=(register_ebp - (int)current()) + (int)(uchild);

  uchild->task.kernel_esp=register_ebp + sizeof(DWord);
 
  DWord temp_ebp = *(DWord*)register_ebp;
  /* Prepare child stack for context switch */
  uchild->task.kernel_esp-=sizeof(DWord);
  *(DWord*)(uchild->task.kernel_esp)=(DWord)&ret_from_fork;
  uchild->task.kernel_esp-=sizeof(DWord);
  *(DWord*)(uchild->task.kernel_esp)=temp_ebp;

  INIT_LIST_HEAD(&(schild->child_list));
  list_add( &(schild->anchor), &current()->child_list);
 
  schild->PID=pids++;
  schild->quantum=DEFAULT_QUANTUM;

  struct list_head *it; 
  list_for_each( it, &current()->child_list ) {
    struct task_struct *child = list_head_to_task_struct(it);  
    char *buffer = "\0\0\0\0\0";
    itoa(child->PID, buffer);
    printk("\n");  printk("child_list pid ");
    printk(buffer);
 } 
 
  /* Queue child process into readyqueue */
  list_add_tail(&(schild->list), &readyqueue);
  
  return uchild->task.PID;
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
/*	    char * buffer = "\0\0\0\0\0";
        itoa(pcb_child->PID, buffer);
       printk(buffer); printk("\n");
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

