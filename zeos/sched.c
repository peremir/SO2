/*
 * sched.c - initializes struct for task 0 anda task 1
 */

#include <sched.h>
#include <mm.h>
#include <io.h>

struct list_head  freequeue;
struct list_head readyqueue;

struct task_struct * idle_task;
struct task_struct * init_task;

union task_union task[NR_TASKS]
  __attribute__((__section__(".data.task")));

#if 1
struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry( l, struct task_struct, list);
}
#endif

extern struct list_head blocked;

int pids;

/* get_DIR - Returns the Page Directory address for task 't' */
page_table_entry * get_DIR (struct task_struct *t) 
{
  return t->dir_pages_baseAddr;
}

/* get_PT - Returns the Page Table address for task 't' */
page_table_entry * get_PT (struct task_struct *t) 
{
  return (page_table_entry *)(((unsigned int)(t->dir_pages_baseAddr->bits.pbase_addr))<<12);
}

int allocate_DIR(struct task_struct *t) 
{
  int pos;

  pos = ((int)t-(int)task)/sizeof(union task_union);

  t->dir_pages_baseAddr = (page_table_entry*) &dir_pages[pos]; 

  return 1;
}

void cpu_idle(void)
{
  __asm__ __volatile__("sti": : :"memory");

  while(1)
  {/*
   printk(" cpu_idle with PID: ");
   char *buffer2 = "\0\0\0\0\0";
   itoa(current()->PID, buffer2);
   printk(buffer2); */
  ;
  }
}

void init_idle (void) 
{
  struct list_head *free = list_first(&freequeue);
  list_del(free);
  union task_union *pcb = (union task_union*)list_head_to_task_struct(free);
  
  pcb->task.PID = 0; 
  allocate_DIR(&(pcb->task));
  //init stats

  pcb->stack[KERNEL_STACK_SIZE - 1] = (unsigned long)cpu_idle;
  pcb->stack[KERNEL_STACK_SIZE - 2] = 0;
  pcb->task.kernel_esp = &(pcb->stack[KERNEL_STACK_SIZE - 2]);
  
  idle_task = (struct task_struct*)pcb;
}

void init_task1(void) // task1 = INIT
{
  struct list_head *free = list_first(&freequeue);
  list_del(free);
  union task_union *pcb = (union task_union*)list_head_to_task_struct(free);

  pcb->task.PID = 1; 
  allocate_DIR(&(pcb->task));
  set_user_pages(&(pcb->task));
  //init stats

  pcb->task.kernel_esp = &(pcb->stack[KERNEL_STACK_SIZE]);
  tss.esp0 = (DWord)pcb->task.kernel_esp;
  writeMSR(0x175, tss.esp0);

  set_cr3(get_DIR(&(pcb->task)));

  init_task = (struct task_struct*)pcb; 
}

void inner_task_switch(union task_union * new) { 
  tss.esp0 = KERNEL_ESP(new);
  writeMSR(0x175, tss.esp0);
	
  set_cr3(get_DIR((struct task_struct*)new));
  current()->kernel_esp = get_ebp();

  set_esp(new->task.kernel_esp);
}

void init_sched()
{
  pids = 1;

  INIT_LIST_HEAD(&freequeue);
  INIT_LIST_HEAD(&readyqueue);

  for (int i = 0; i < NR_TASKS; i++) {
    list_add_tail(&(task[i].task.list),&freequeue);
  }
}

struct task_struct* current()
{
  int ret_value;
  
  __asm__ __volatile__(
  	"movl %%esp, %0"
	: "=g" (ret_value)
  );
  return (struct task_struct*)(ret_value&0xfffff000);
}

