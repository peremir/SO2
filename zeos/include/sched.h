/*
 * sched.h - Estructures i macros pel tractament de processos
 */

#ifndef __SCHED_H__
#define __SCHED_H__

#include <list.h>
#include <types.h>
#include <mm_address.h>

#define NR_TASKS      10
#define KERNEL_STACK_SIZE	1024

#define DEFAULT_QUANTUM 10
#define MAX_MUTEXES 10

struct task_struct {
  int PID;			/* Process ID. This MUST be the first field of the struct. */
  int quantum;                  /* ZeOS Ticks remaining */
  int kernel_esp;             /* kernel stack pointer */
  struct list_head list;        /* Which list is queued */
  page_table_entry * dir_pages_baseAddr;
  struct list_head child_list;  //Llista de tots els fills
  struct list_head anchor;      //Anchor per si el proces es fill d'algun pare
  struct task_struct *parent;
  int circ_buff_chars_to_read;
  int circ_buff_maxchars;

};

union task_union {
  struct task_struct task;
  unsigned long stack[KERNEL_STACK_SIZE];    /* pila de sistema, per procés */
};

struct mutex_t {
    int count;
    struct list_head blocked_queue;
};


extern union task_union task[NR_TASKS]; /* Vector de tasques */
extern struct mutex_t mutexes[MAX_MUTEXES];

extern struct list_head freequeue;
extern struct list_head readyqueue;
extern struct task_struct * idle_task;

#define KERNEL_ESP(t)       	(DWord) &(t)->stack[KERNEL_STACK_SIZE]

#define INITIAL_ESP       	KERNEL_ESP(&task[1])

void inner_task_switch (union task_union *t);

extern int pids;
extern int quantum_left;
extern int pending_unblocks;

/* Inicialitza les dades del proces inicial */
void init_task1(void);

void init_idle(void);

void init_sched(void);

struct task_struct * current();

void task_switch(union task_union*t);

void inner_task_switch(union task_union * new);

struct task_struct *list_head_to_task_struct(struct list_head *l);

int allocate_DIR(struct task_struct *t);

page_table_entry * get_PT (struct task_struct *t) ;

page_table_entry * get_DIR (struct task_struct *t) ;

/* Headers for the scheduling policy */
void sched_next_rr();
void update_process_state_rr(struct task_struct *t, struct list_head *dest);
int needs_sched_rr();
void update_sched_data_rr();
void schedule();

struct mutex_t* mutex_get(int id);

#endif  /* __SCHED_H__ */
