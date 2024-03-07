/*
 * io.c - 
 */

#include <io.h>

#include <types.h>

/**************/
/** Screen  ***/
/**************/

#define NUM_COLUMNS 80
#define NUM_ROWS    25

Byte x, y=19;



/* Read a byte from 'port' */
Byte inb (unsigned short port)
{
  Byte v;

  __asm__ __volatile__ ("inb %w1,%0":"=a" (v):"Nd" (port));
  return v;
}

//
void scroll()
{
  Word *screen = (Word *)0xb8000;
  // loop through the screen content except the last row
  for (int i = 0; i < (NUM_ROWS - 1) * NUM_COLUMNS; i++) 
  {
    // copy the content of the next row to the current row
    screen[i] = screen[i + NUM_COLUMNS];
  }
  // loop through the last row
  for (int i = (NUM_ROWS - 1) * NUM_COLUMNS; i < NUM_ROWS * NUM_COLUMNS; i++)
  {
    // clear the content of the last row
    screen[i] = 0x0200;
  }
}

//OPTIONAL Modified printc function to scroll text when reaches bottom of screen
void printc(char c)
{
     __asm__ __volatile__ ( "movb %0, %%al; outb $0xe9" ::"a"(c)); /* Magic BOCHS debug: writes 'c' to port 0xe9 */
  if (c=='\n')
  {
    x = 0;
    y=(y+1)%NUM_ROWS;
    
    // if the cursor reaches the last row
    if (y >= NUM_ROWS-1)
    {
      // call the scroll function and adjuts the cursos position y--
      scroll();
      y--;
    }
  }
  else
  {
    Word ch = (Word) (c & 0x00FF) | 0x0200;
	Word *screen = (Word *)0xb8000;
	screen[(y * NUM_COLUMNS + x)] = ch;
    if (++x >= NUM_COLUMNS)
    {
      x = 0;
      y=(y+1)%NUM_ROWS;
      
      // if the cursor reaches the last row
      if (y >= NUM_ROWS-1)
      {
	// call the scroll function and adjuts the cursos position y--
        scroll(); 
        y--; 
      }
    }
  }
}

//OPTIONAL Modified printc color to print foreground in a different color
void printc_color(char c)
{
     __asm__ __volatile__ ( "movb %0, %%al; outb $0xe9" ::"a"(c)); /* Magic BOCHS debug: writes 'c' to port 0xe9 */
  if (c=='\n')
  {
    x = 0;
    y=(y+1)%NUM_ROWS;
  }
  else
  {
    /*                    in 0x00FF | 0xN200, the first N byte is the color
    0=Black, 1=Blue, 2=Green, 3=Aqua, 4=Red, 5=Purple, 6=Yellow, 7=White, 8=Gray, 9=Light Blue, 
    A=Light Green, B=Light Aqua, C=Light Red, D=Light Purple, E=Light Yellow, F=Bright White */

    Word ch = (Word) (c & 0x00FF) | 0x5200;
    Word *screen = (Word *)0xb8000;
    
    /* y= adalt_abaix   x= dreta_esquerra */
    screen[(y * NUM_COLUMNS + x)] = ch;
    
    if (++x >= NUM_COLUMNS)
    {
      x = 0;
      y=(y+1)%NUM_ROWS;
    }
  }
}




void printcolor(char c)
{
     __asm__ __volatile__ ( "movb %0, %%al; outb $0xe9" ::"a"(c)); /* Magic BOCHS debug: writes 'c' to port 0xe9 */
  if (c=='\n')
  {
    x = 0;
    y=(y+1)%NUM_ROWS;
  }
  else
  {
    Word ch = (Word) (c & 0x00FF) | 0x8d00;
	Word *screen = (Word *)0xb8000;
	screen[(y * NUM_COLUMNS + x)] = ch;
    if (++x >= NUM_COLUMNS)
    {
      x = 0;
      y=(y+1)%NUM_ROWS;
    }
  }
}




void printc_xy(Byte mx, Byte my, char c)
{
  Byte cx, cy;
  cx=x;
  cy=y;
  x=mx;
  y=my;
  printc(c);
  x=cx;
  y=cy;
}

void printk(char *string)
{
  int i;
  for (i = 0; string[i]; i++)
    printc(string[i]);
}

//Modified printk function to call printc_color()
void printk_color(char *string)
{
  int i;
  for (i = 0; string[i]; i++)
    printc_color(string[i]);
}

