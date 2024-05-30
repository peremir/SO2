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

#include <interrupt.h>

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
    set_ss_pag(parent_PT, pag+NUM_PAG_DATA+NUM_PAG_CODE+5, get_frame(process_PT, pag));
    copy_data((void*)(pag<<12), (void*)((pag+NUM_PAG_DATA+NUM_PAG_CODE+5)<<12), PAGE_SIZE);
    del_ss_pag(parent_PT, pag+NUM_PAG_DATA+NUM_PAG_CODE+5);
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
/*
  struct list_head *it; 
  list_for_each( it, &current()->child_list ) {
    struct task_struct *child = list_head_to_task_struct(it);  
    char *buffer = "\0\0\0\0\0";
    itoa(child->PID, buffer);
    printk("\n");  printk("child_list pid ");
    printk(buffer);
 } */
 
  /* Queue child process into readyqueue */
  list_add_tail(&(schild->list), &readyqueue);
  
  return uchild->task.PID;
}

void sys_exit()
{ /*
  struct task_struct* current_proc = current();

  pcbs_in_dir[get_DIR_pos(current_proc)]--;

  if (pcbs_in_dir[get_DIR_pos(current_proc)] == 0) 
  {
    // Free user pages
    free_user_pages(current_proc);
  }
  list_add_tail(&current_proc->list, &freequeue);

  sched_next_rr();
	*/	
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


int is_blocked(struct task_struct * p) {
    struct list_head * it;
    list_for_each(it, &blocked) {
        struct task_struct* bl_process = list_head_to_task_struct(it);
        if (&bl_process->list == &p->list) {
            return 1;
        }
    }
    return 0;
}


int sys_unblock(int pid)
{
  struct list_head* it;
  list_for_each(it, &(current()->child_list))
  {
    struct task_struct *pcb_child = list_head_to_task_struct(it);	
    if (pcb_child->PID == pid) 
    {
      //if (list_first(&(pcb_child->list)) == list_first(&blocked)) 
      if (is_blocked(pcb_child))
      {
        //desbloquearlo
        struct list_head * l = &(pcb_child->list);
        list_del(l);
        list_add_tail(l, &readyqueue);
        return 0;
      }
      else 
      {
        pending_unblocks++;
        return 0;
      }
    }
  }
    //ESTO TIENE QUE SER -1 !!!!
   return -1;
}


int sys_read(char *b, int maxchars) 
{
  if (maxchars <= 0) 
    return -EINVAL;
  
  if (!access_ok(VERIFY_WRITE, b, maxchars)) 
    return -EFAULT;
    
  struct task_struct *t = current();

  t->circ_buff_chars_to_read = maxchars;
  t->circ_buff_maxchars = maxchars;

  // poner proceso en blocked prk scheduler no el psoi
  update_process_state_rr(t, &readblocked);
    
  int diff = t->circ_buff_maxchars - t->circ_buff_chars_to_read;
  char buff[TAM_BUF];
  while (t->circ_buff_chars_to_read > 0) 
  {
    sched_next_rr();

    int i = 0;
    char c = circ_buff_read();
    while (c != '\0') 
    {
      buff[i] = c;
      ++i;
      c = circ_buff_read();
    }
    copy_to_user(buff, b + diff, i);
    diff = t->circ_buff_maxchars - t->circ_buff_chars_to_read;
  }
  copy_to_user((void*)"\0", b+diff, 1);
  update_process_state_rr(t,NULL);  
  
  return maxchars;
}


int sys_create_thread(void (*start_routine)(void* arg), void *parameter) 
{
  if (list_empty(&freequeue)) 
    return -ENOMEM;

  if (!access_ok(VERIFY_READ, start_routine, sizeof(void*))) 
    return -EFAULT;
  //pillar pcb, delete, taskstruct-union etc.. la vaina
  struct list_head *free = list_first(&freequeue);
  list_del(free);

  struct task_struct* pcb = list_head_to_task_struct(free);
  union task_union* pcb_union = (union task_union*)pcb;
  
  //copiem data de current a pcb pilla  
  copy_data(current(), pcb_union, sizeof(union task_union));
  
  //posicio dir de pagines del thread i sumar contador
  int dir_pos = ((int)get_DIR(pcb)-(int)dir_pages)/(sizeof(page_table_entry)*TOTAL_PAGES);
  pcbs_in_dir[dir_pos]++;
  
  //stack fill
  DWord *new_stack = get_new_stack(get_PT(pcb));
  if (new_stack == NULL) 
    return -ENOMEM;   //ENOMEM sirve
  
  //BASE KERNEL STACK ADDRESS
  DWord *base_stack = &(pcb_union->stack[KERNEL_STACK_SIZE]);
 
  //Primer i segon DWord size param 
  base_stack[-1] = (DWord)parameter;
  base_stack[-2] = (DWord)0; 

  int register_ebp;             /* frame pointer */
  /* Map Parent's ebp to child's stack */
  register_ebp = (int) get_ebp();
  register_ebp=(register_ebp - (int)current()) + (int)(pcb_union);

  pcb_union->task.kernel_esp=register_ebp + sizeof(DWord);

  DWord temp_ebp = *(DWord*)register_ebp;
  /* Prepare child stack for context switch */
  pcb_union->task.kernel_esp-=sizeof(DWord);
  *(DWord*)(pcb_union->task.kernel_esp)=(DWord)&ret_from_fork;
  pcb_union->task.kernel_esp-=sizeof(DWord);
  *(DWord*)(pcb_union->task.kernel_esp)=temp_ebp;

  /* Adress where el thread ha de comen+ar */ 
  base_stack[-5] = (DWord)start_routine;
  base_stack[-2] = (DWord)&new_stack[(PAGE_SIZE/4)-2];

  list_add_tail(free, &readyqueue);

  return 0; 
}

void sys_exit_thread(void) 
{
  struct task_struct* t = current();
    
  union task_union* t_union = (union task_union*)t;
  
  //BOrrar paginess
  DWord *base_stack = &(t_union->stack[KERNEL_STACK_SIZE]);
  DWord page = base_stack[-2] >> 12;  //mem pag de la stack
  del_ss_pag(get_PT(t), page);

  struct task_struct* current_proc = current();

  pcbs_in_dir[get_DIR_pos(current_proc)]--;
 
  if (pcbs_in_dir[get_DIR_pos(current_proc)] == 0) 
   {
     // Free user pages
     free_user_pages(current_proc);
}  

  list_add_tail(&current_proc->list, &freequeue);

  sched_next_rr();
}


int sys_mutex_init(int *m) 
{
  if (!access_ok(VERIFY_READ, m, sizeof(int)))
    return -EFAULT;

  int m_sys = 0;
  copy_from_user(m, &m_sys, sizeof(int));

  struct mutex_t *mutex = mutex_get(m_sys);
  if (mutex == NULL) 
    return -1;

  mutex->count = 0;

  return 0;
}

int sys_mutex_lock(int *m) 
{
  if (!access_ok(VERIFY_WRITE, m, sizeof(int)) || !access_ok(VERIFY_READ, m, sizeof(int))) 
    return -EFAULT;

  int m_sys;
  copy_from_user(m, &m_sys, sizeof(int));

  struct mutex_t *mutex = mutex_get(m_sys);
  if (mutex == NULL || mutex->count == -1) 
    return -1;

  if (mutex->count >= 1) 
  {     
    update_process_state_rr(current(), &mutex->blocked_queue);
    sched_next_rr();
  }
  mutex->count++;

  return 0;
}

int sys_mutex_unlock(int *m) 
{
  if (!access_ok(VERIFY_WRITE, m, sizeof(int)) || !access_ok(VERIFY_READ, m, sizeof(int)))
    return -EFAULT;

  int m_sys;
  copy_from_user(m, &m_sys, sizeof(int));

  struct mutex_t *mutex = mutex_get(m_sys);
  if (mutex == NULL || mutex->count == -1) 
    return -1;

  if (mutex->count == 0) 
    return 0;

  if (!list_empty(&mutex->blocked_queue)) 
  {
    struct task_struct *t = list_head_to_task_struct(list_first(&mutex->blocked_queue));
    list_del(&t->list);
    list_add(&t->list, &readyqueue);
  }
  mutex->count--;

  return 0;
}

