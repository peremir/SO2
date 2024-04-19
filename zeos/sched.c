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
extern int quantum_left;

int pids;

int get_quantum(struct task_struct *t) {
    return t->quantum;
}

void set_quantum(struct task_struct *t, int new_quantum) {
    t->quantum = new_quantum;
}


union task_union task[NR_TASKS]
  __attribute__((__section__(".data.task")));

#if 1
struct task_struct *list_head_to_task_struct(struct list_head *l)
{
  return list_entry( l, struct task_struct, list);
}
#endif

extern struct list_head blocked;


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
	printk("funca");

	char *buffer = "\0\0\0\0\0\n";
  	itoa(current()->PID, buffer);
	printk(buffer);


	while(1) 
	{
	;
	}
}

void init_stats(struct task_struct *t) {
    t->stats.user_ticks = 0;
    t->stats.system_ticks = 0;
    t->stats.ready_ticks = 0;
    t->stats.blocked_ticks = 0;
    t->stats.elapsed_total_ticks = get_ticks();
    t->stats.total_trans = 0;
    t->stats.remaining_ticks = 0;
}

void init_idle (void) 
{
  // 1) Get an available task_union from the freequeue 
  //  to contain the characteristics of this process.
  struct list_head *free = list_first(&freequeue);
  list_del(free);
  union task_union *pcb = (union task_union*)list_head_to_task_struct(free);
  
  //2) Assign PID 0 to the process. 
  pcb->task.PID = 0; 
  pids++; 
  pcb->task.quantum = DEFAULT_QUANTUM;
  init_stats((struct task_struct*)pcb);

  // 3) Initialize field dir_pages_baseAaddr with a new d:irectory 
  //  to store the process address space using the allocate_DIR routine.
  allocate_DIR(&(pcb->task));

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
  pids++;
  allocate_DIR(&(pcb->task));
  pcb->task.quantum = DEFAULT_QUANTUM;
  quantum_left = pcb->task.quantum;
  init_stats((struct task_struct*)pcb);
  set_user_pages(&pcb->task);

  pcb->task.kernel_esp = &(pcb->stack[KERNEL_STACK_SIZE]);
  tss.esp0 = (DWord)pcb->task.kernel_esp;
  writeMSR(0x175, tss.esp0);

  set_cr3(get_DIR(&(pcb->task)));


  init_task = (struct task_struct*)pcb; 
}

void inner_task_switch(union task_union * new) {
	tss.esp0 = KERNEL_ESP(new);
	writeMSR(0x175, tss.esp0);
	
	set_cr3(get_DIR(&(new->task)));
	DWord ebp = get_ebp();
	current()->kernel_esp = ebp;

	set_esp((struct task_struct *)new->task.kernel_esp);


}

void update_sched_data_rr() {
	--quantum_left;
}

int needs_sched_rr() {
	if ((quantum_left <= 0) && !list_empty(&readyqueue)) return 1;
	if ((quantum_left <= 0)) quantum_left = current()->quantum;
	return 0;
}

void update_process_state_rr(struct task_struct *t, struct list_head *dst_queue) {
	if (t != current() && t != idle_task) list_del(&t->list);
    	if (dst_queue != NULL) list_add_tail(&t->list, dst_queue);
}




void sched_next_rr() {
    struct task_struct *next_task;

    if (list_empty(&readyqueue)) 
        next_task = idle_task;
    else
        next_task = list_head_to_task_struct(list_first(&readyqueue));

    update_process_state_rr(next_task, NULL);

    task_switch((union task_union*)next_task);
}

void init_sched()
{
  pids = 0;
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

