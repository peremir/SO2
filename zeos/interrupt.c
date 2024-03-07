/*
 * interrupt.c -
 */
#include <types.h>
#include <interrupt.h>
#include <segment.h>
#include <hardware.h>
#include <io.h>

#include <zeos_interrupt.h>

Gate idt[IDT_ENTRIES];
Register    idtR;
unsigned int zeos_ticks = 0;

void hank();
void clank();
void page_fault_exception_handler();
void system_call_handler();

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

//flags |= 0x8F00;    /* P = 1, D = 1, Type = 1111 (Trap Gate) */
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
  setInterruptHandler(33, hank, 0);	/* Keyboard interrupt */
  setInterruptHandler(32, clank, 0);	/* Clock interrupt */
  setInterruptHandler(14, page_fault_exception_handler, 0);

  setTrapHandler(0x80,system_call_handler,3);
  

  set_idt_reg(&idtR);
}


void keyboardService()
{
  unsigned char key = inb(0x60);

  if((key & 0x80) != 0x80)
  {
    if(char_map[key] != '\0')
    { 
      printc_xy(0,0,char_map[key]);
    }
    else
    {
      printc_xy(0,0,'C');
    }
  }
}


void clock_routine() {
	zeos_ticks++;
	zeos_show_clock();	
}

void clockRoutine()
{
  zeos_show_clock(); 
}



void pf_routine(int error, int eip) {
//	necesito la adreça on ha fallat la pinga

	char hexChars[] = "0123456789ABCDEF";
    	char hex[6]; // 5 caracteres para el valor hexadecimal más el terminador nulo

    	for (int i = 0; i < 5; ++i) {
        	hex[i] = hexChars[(eip >> (16 - i * 4)) & 0xF];
    	}
    	hex[5] = '\0'; // Asegurarse de que la cadena esté terminada correctamente

	printk("\nProcess generates a PAGE FAULT exception at EIP: 0x");
	printk(hex);
	printk("\n\n");
	while(1);
}


