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

Byte x, y;


/* Read a byte from 'port' */
Byte inb (unsigned short port)
{
  Byte v;

  __asm__ __volatile__ ("inb %w1,%0":"=a" (v):"Nd" (port));
  return v;
}

//NEW Scroll up the screen by one row
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

//NEW change pointer location
void change_pointer(Byte a, Byte b)
{
  x = a;
  y = b;
}

//NEW Clear the entire screen to make it fully black
void clear_screen(int mask)
{
  
  Word *screen = (Word *)0xb8000;
  
  Word attribute = mask; 
  // Color de fondo rojo
  
  Word empty_character = attribute | ' '; 
  // Carácter de espacio con fondo rojo
  
  // Loop through all rows and columns of the screen
  for (int row = 0; row < NUM_ROWS; row++) {
    for (int col = 0; col < NUM_COLUMNS; col++) {
      // Clear each character on the screen
      screen[row * NUM_COLUMNS + col] = empty_character;
    }
  }
  // Reset the cursor position to (0,0)
  x = 0;
  y = 0;
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
void printc_color(char c, int mask)
{
     __asm__ __volatile__ ( "movb %0, %%al; outb $0xe9" ::"a"(c)); 
     /* Magic BOCHS debug: writes 'c' to port 0xe9 */
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
    Word ch = (Word) (c & 0x00FF) | mask;
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
void printk_color(char *string, int mask)
{
  int i;
  for (i = 0; string[i]; i++)
    printc_color(string[i], mask);
}

