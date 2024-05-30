/*
 * interrupt.c -
 */
#include <types.h>
#include <interrupt.h>
#include <segment.h>
#include <hardware.h>
#include <io.h>
#include <libc.h>
#include <entry.h>
#include <sched.h>
#include <devices.h>

#include <zeos_interrupt.h>

Gate idt[IDT_ENTRIES];
Register    idtR;
unsigned int zeos_ticks = 0;

void keyboardHandler();
void clockHandler();
void system_call_handler();
void syscall_handler_sysenter();
void page_fault_exception_handler();

char char_map[] =
{
  '\0','\0','1','2','3','4','5','6',
  '7','8','9','0','\'','¡','\0','\0',
  'q','w','e','r','t','y','u','i',
  'o','p','`','+','\0','\0','a','s',
  'd','f','g','h','j','k','l','ñ',
  '\0','º','\0','ç','z','x','c','v',
  'b','n','m',',','.','-','\0','*',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','7',
  '8','9','-','4','5','6','+','1',
  '2','3','0','\0','\0','\0','<','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0'
};
;


void setInterruptHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE INTERRUPTION GATE FLAGS:                          R1: pg. 5-11  */
  /* ***************************                                         */
  /* flags = x xx 0x110 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}

void setTrapHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE TRAP GATE FLAGS:                                  R1: pg. 5-11  */
  /* ********************                                                */
  /* flags = x xx 0x111 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);

  /* flags |= 0x8F00;    // P = 1, D = 1, Type = 1111 (Trap Gate) */
  /* Changed to 0x8e00 to convert it to an 'interrupt gate' and so
   the system calls will be thread-safe. */
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}


void setIdt()
{
  /* Program interrups/exception service routines */
  idtR.base  = (DWord)idt;
  idtR.limit = IDT_ENTRIES * sizeof(Gate) - 1;

  set_handlers();

  /* ADD INITIALIZATION CODE FOR INTERRUPT VECTOR */
  writeMSR(0x174, __KERNEL_CS);
  writeMSR(0x175, INITIAL_ESP);
  writeMSR(0x176, (int)syscall_handler_sysenter);

  setInterruptHandler(33, keyboardHandler, 0);  /* Keyboard interrupt */
  setInterruptHandler(32, clockHandler, 0); /* Clock interrupt */
  setInterruptHandler(14, page_fault_exception_handler, 0);

  setTrapHandler(0x80,system_call_handler,3);


  set_idt_reg(&idtR);
}

void keyboardService () 
{
  char key, c;
  key = inb(0x60); // Llegim el regisre

  //make
  if ((key & 0x80) == 0x80)  
    return;

  c = char_map[key];
  if (c == '\0') 
    c = 'C';

  printc_xy(0, 0, c);

  circ_buff_append(c);
  // if no processes are blocked waiting for the keyboard input, nothing more needs to be done
  if (list_empty(&readblocked) || c == 'p') 
    return;

  struct list_head *l = list_first(&readblocked);
  struct task_struct *t = list_head_to_task_struct(l);

  if (t->circ_buff_chars_to_read > 0) 
  {
     if (t->circ_buff_maxchars == t->circ_buff_chars_to_read) {
     //list_add_tail(current(), &readyqueue);
     update_process_state_rr(current(), &readyqueue);
     }

     //t->circ_buff_chars_to_read--;

    // mirar si buffer lleno
    //if (t->circ_buff_chars_to_read == 0) 
    /*{
      return 0;
    }*/

    //list_add(t, &readyqueue); //FALTA POSAR AL PRNCIPI
      //sched_next_rr();
	task_switch((union task_union *)t);
  } 
} 


void clockRoutine() 
{
  zeos_ticks++;
  zeos_show_clock();

  schedule();  
}

//NEW versio only text
void pf_routine(int error, int eip) 
{  
  char *text = "\nProcess generates a PAGE FAULT exception at EIP: @";
  printk(text);
  
  char char_eip[24];
  itoa(eip, char_eip);
  printk(char_eip);		
  
  char hex[9]; // 5 caracteres para el valor hexadecimal más el terminador nulo
  int decimal = eip;
  int nonzero = 0;
    for (int i = 0; i < 8; ++i) 
    {
      char digit = "0123456789ABCDEF"[decimal & 0xF];
      if (digit != '0' || nonzero) 
      {
        hex[7 - i] = digit;
        nonzero = 1;
      } 
      else 
      {
        hex[7 - i] = '0';
      }
      decimal >>= 4;
    }
  
  // Terminador nulo
  hex[8] = '\0'; //Asegurarse de que la cadena esté terminada correctamente
  
  printk(" (0x");
  printk(hex);
  printk(")");

  while(1);
}

/*
void pf_red_screen(char *eip, char *hex)
{
  clear_screen(0x4000);
  
  change_pointer(0, 8);

  printk_color("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", 0x0400);
  printk_color("!!                                                     !!\n", 0x0400);
  printk_color("!!  !!! Process generates a PAGE FAULT exception !!!   !!\n", 0x0400);

  printk_color("!!             at EIP: @", 0x0400); 
  printk_color(eip , 0x0400);
  printk_color(" (0x", 0x0400);
  printk_color(hex , 0x0400);
  printk_color(")              !!\n", 0x0400);
  
  printk_color("!!                                                     !!\n", 0x0400);
  printk_color("!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n", 0x0400);
} */

/*
//NEW versio amb page fault red screen
void pf_routine(int error, int eip) 
{
  // necesito la adreça on ha fallat la pinga
  
  char char_eip[24];
  itoa(eip, char_eip);
  printk(char_eip);

  char hexChars[] = "0123456789ABCDEF";
  char hex[6]; // 5 caracteres para el valor hexadecimal más el terminador nulo

  for (int i = 0; i < 5; ++i) 
  {
    hex[i] = hexChars[(eip >> (16 - i * 4)) & 0xF];
  }
  hex[5] = '\0'; // Asegurarse de que la cadena esté terminada correctamente
  
  pf_red_screen(char_eip, hex);
  
  while(1);
}
*/

char circ_buffer[TAM_BUF];
char *circ_buff_head = &circ_buffer[0];
char *circ_buff_tail = &circ_buffer[0];
int circ_buff_num_items = 0;


char circ_buff_append(char c) 
{
  if (circ_buff_is_full()) 
  {
    return -1;
  }

  *circ_buff_head = c;
  circ_buff_head++;
  circ_buff_num_items++;

  if (circ_buff_head == &circ_buffer[TAM_BUF]) 
  {
      circ_buff_head = &circ_buffer[0];
  }

  return 1;
}

char circ_buff_read() 
{
  // se mira esta vacio
  if (circ_buff_num_items == 0) 
  {
    return '\0';
  }

  char c = *circ_buff_tail;

  circ_buff_tail++;
  circ_buff_num_items--;

  if (circ_buff_tail == &circ_buffer[TAM_BUF]) 
  {
    circ_buff_tail = &circ_buffer[0];
  }

  return c;
}

char circ_buff_is_full() 
{
  return circ_buff_num_items == TAM_BUF;
}

