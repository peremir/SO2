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
	if (pcb_child->PID == pid) 
	{
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
    return -1;
}


int sys_read(char *b, int maxchars) {
    if (maxchars <= 0) return EINVAL;
    if (!access_ok(VERIFY_WRITE, b, maxchars)) return EFAULT;
    
    struct task_struct *t = current();

    t->circ_buff_chars_to_read = maxchars;
    t->circ_buff_maxchars = maxchars;

    // poner proceso en blocked para que el scheduler no le pille.
    update_process_state_rr(t, &blocked);
    
    int diff = t->circ_buff_maxchars - t->circ_buff_chars_to_read;
    char buff[TAM_BUF];
    while (t->circ_buff_chars_to_read > 0) {
        sched_next_rr();

        int i = 0;
        char c = circ_buff_read();
        while (c != '\0') {
            buff[i] = c;
            ++i;
            c = circ_buff_read();
        }

        copy_to_user(buff, b + diff, i);

        diff = t->circ_buff_maxchars - t->circ_buff_chars_to_read;
    }

    copy_to_user((void*)"\0", b+diff, 1);

    update_process_state_rr(current(), &readyqueue);
    sched_next_rr();

    return maxchars;
}


int sys_create_thread(void (*start_routine)(void* arg), void *parameter) {
    if (list_empty(&freequeue)) {
        return ENOMEM;
    }

    if (!access_ok(VERIFY_READ, start_routine, sizeof(void*))) {
        return EFAULT;
    }

    // if (!access_ok(VERIFY_READ, parameter, sizeof(void*))) {
    //     return EFAULT;
    // }

    struct list_head *free_list_pos = list_first(&freequeue);
    list_del(free_list_pos);

    struct task_struct* pcb = list_head_to_task_struct(free_list_pos);
    union task_union* pcb_union = (union task_union*)pcb;

    copy_data(current(), pcb_union, sizeof(union task_union));

    int dir_pos = ((int)get_DIR(pcb)-(int)dir_pages)/(sizeof(page_table_entry)*TOTAL_PAGES);
    pcbs_in_dir[dir_pos]++;

    DWord *new_stack = get_new_stack(get_PT(pcb));
    if (new_stack == NULL) return ENOMEM;

    DWord *base_stack = &(pcb_union->stack[KERNEL_STACK_SIZE]);

    new_stack[(PAGE_SIZE/4)-1] = (DWord)parameter;
    new_stack[(PAGE_SIZE/4)-2] = (DWord)0; // evil floating point bit level hacking

    pcb->kernel_esp = &(pcb_union->stack[KERNEL_STACK_SIZE-19]); // 17-2 por el @ret_from_fork y el ebp.
    pcb_union->stack[KERNEL_STACK_SIZE-19] = 0;
    pcb_union->stack[KERNEL_STACK_SIZE-18] = (DWord)ret_from_fork;

    base_stack[-5] = (DWord)start_routine;
    base_stack[-2] = (DWord)&new_stack[(PAGE_SIZE/4)-2];

    list_add_tail(free_list_pos, &readyqueue);

    return 0;
}

void sys_exit_thread(void) {
    struct task_struct* t = current();

    // TODO reemplazar esto con bithack raro para pillar
    // la página del esp. Esto borra todo lo que hay después de
    // las páginas de código en la tabla de páginas.
    // del_ss_extra_pages(get_PT(t));

    union task_union* t_union = (union task_union*)t;
    DWord *base_stack = &(t_union->stack[KERNEL_STACK_SIZE]);

    DWord page = base_stack[-2] >> 12;

    del_ss_pag(get_PT(t), page);

    sys_exit();
}

/*
int sys_threadCreateWithStack(void (*function)(void* arg), int N, void* parameter) {
// 1er, comprovem paràmetres
  if (N <= 0) // N > 0, perquè si no, no es pot fer crida
  	return -EINVAL;
  
  if (function == NULL) // La funció ha d'apuntar a algun lloc (!= NUll)
  	return -EINVAL;
  
  
  struct list_head *lhcurrent = NULL;
  union task_union *uchild;
  
  // Any free task_struct? 
  if (list_empty(&freequeue)) return -ENOMEM;

  lhcurrent=list_first(&freequeue);
  
  list_del(lhcurrent);
  
  uchild=(union task_union*)list_head_to_task_struct(lhcurrent);
  
  // Copy the parent's task struct to child's 
  copy_data(current(), uchild, sizeof(union task_union)); // això inclou posicions pila usuari (rang; data és a taula pàg.)!
  
  // Ara cerquem en la taula de pàgines espai per la pila de N posicions, i en cas que sí, veiem si podem reservar N frames de la memòria física per la pila (ho farem des d'abaix, perquè la part de dalt l'utilitzi mem. dinàmica) 
  page_table_entry *process_PT = get_PT(current()); // (és mateixa per thread pare i fill)
  int pag, i;
  for (pag = TOTAL_PAGES-1; pag-N >= -1;){
  
    char ocupat = 0;
    for (i = pag; !ocupat && i > pag-N; --i){ // mirem N pàgines consecutives
    	if ( has_frame(process_PT, i) )
    		ocupat = 1;
    }
    		
    if (!ocupat){ // espai lliure, intentem reservar-hi frames pila
    	int new_ph_pag;
    
    	for (i = pag; i > pag-N; --i)
	  {
	    new_ph_pag=alloc_frame();
	    if (new_ph_pag!=-1) // One page allocated 
  // Any free task_struct?
  if (list_empty(&freequeue)) return -ENOMEM;

  lhcurrent=list_first(&freequeue);

  list_del(lhcurrent);
	    {
	      set_ss_pag(process_PT, i, new_ph_pag);
	    }
	    else // No more free pages left. Deallocate everything 
	    {
	      // Deallocate allocated pages. Up to pag. 
	      for (int j = pag; j > i; --j)
	      {
		free_frame(get_frame(process_PT, j));
		del_ss_pag(process_PT, j);
	      }
	      // Deallocate task_struct
	      list_add_tail(lhcurrent, &freequeue);
	      
	      // Return error
	      return -EAGAIN; 
	    }
	  }
	// Si aquí, hem aconseguit fer reserva, així que guardem rang pila
	//printk("\nEntra:");
	//char buff[24]; itoa(pag, buff);
	//printk(buff);
	uchild->task.stack_topPage = pag-N+1;
	uchild->task.stack_bottomPage = pag;
	pag = -2; // marca per sortir de bucle superior, indicant que ha estat per pila reservada
	
	  
    }else
    	pag = i; // Si pàgina de les N consecutives ocupada, ens la saltem i seguim mirant a partir d'aquí
  }
  
  if (pag != -2)
  	return -EAGAIN; // si no marca, error; no hi ha entrades suficients en taula de pàg
  
  // Assignem TID i estat threat //
  uchild->task.TID=++global_TID;
  
  // Preparem context hardware (=> modif. valors de regs guardats + pila usuari tingui al que apunten) //
  // PROVAR 1ER SI ENTRA AL WRAPPER AMB AIXÒ (FALTA CONTEXT PILA PEER TASK SWITCH! (I OPTIMITZACIÓ SI MATEIX PROCÉS))
  
  // 1. Pila usuari
  unsigned long * bottom_userStack = (unsigned long *)((uchild->task.stack_bottomPage << 12) + PAGE_SIZE-4);
  *(bottom_userStack-2) = 0;        // Context
  *(bottom_userStack-1) = function; // del
  *bottom_userStack = parameter;    // thread_wrapper (0 és @retorn "dummy")
  
  // 2. Regs de context hardware
  //0x001001f0
  //uchild->stack[KERNEL_STACK_SIZE-5] = (unsigned long) &thread_wrapper; // eip
  //uchild->stack[KERNEL_STACK_SIZE-4] = (unsigned long) &thread_wrapper; // cs (que apuntin a thread_wrapper)
  uchild->stack[KERNEL_STACK_SIZE-5] = 0x00100c80; // eip
  uchild->stack[KERNEL_STACK_SIZE-4] = 0x00100c80; // cs (que apuntin a thread_wrapper)
  
  uchild->stack[KERNEL_STACK_SIZE-2] = (unsigned long) (bottom_userStack - 2 ); // esp
  uchild->stack[KERNEL_STACK_SIZE-1] = (unsigned long) (bottom_userStack - 2 ); // ss (que apuntin a @retorn = 0)
  
  // Perquè cal, guardem el kernel esp del fill perquè pugui executar després si task_switch cap a ell //
  uchild->task.register_esp = (unsigned long) &uchild->stack[KERNEL_STACK_SIZE-18]; // per apuntar a %ebp salvat (i que el set_esp() el recuperi)

  // Queue child process into readyqueue
  list_add_tail(&(uchild->task.list), &readyqueue);
  
  return uchild->task.TID;
} */

