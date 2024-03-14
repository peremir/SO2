/*
 * io.h - Definició de l'entrada/sortida per pantalla en mode sistema
 */

#ifndef __IO_H__
#define __IO_H__

#include <types.h>

/** Screen functions **/
/**********************/

Byte inb (unsigned short port);
void printc(char c);
void printc_xy(Byte x, Byte y, char c);
void printk(char *string);
void printc_color(char c, int mask);
void printk_color(char *string, int mask);

void change_pointer(Byte a, Byte b);
void scroll();
void clear_screen();
#endif  /* __IO_H__ */
