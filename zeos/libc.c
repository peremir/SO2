/*
 * libc.c 
 */

#include <libc.h>
#include <types.h>

#include <errno.h>
int errno;

void itoa(int a, char *b)
{
  int i, i1;
  char c;
  
  if (a==0) { b[0]='0'; b[1]=0; return ;}
  
  i=0;
  while (a>0)
  {
    b[i]=(a%10)+'0';
    a=a/10;
    i++;
  }
  
  for (i1=0; i1<i/2; i1++)
  {
    c=b[i1];
    b[i1]=b[i-i1-1];
    b[i-i1-1]=c;
  }
  b[i]=0;
}

int strlen(char *a)
{
  int i;
  
  i=0;
  
  while (a[i]!=0) i++;
  
  return i;
}

void perror(void)
{
  switch(-errno)
  {
    case ENOSYS: /* Invalid system call number -38 */
      write(1,"\nInvalid system call number",strlen("\nInvalid system call number"));
      break;

    case EFAULT: /* Bad address -14 */
      write(1,"\nBad address",strlen("\nBad address"));
      break;

    case EINVAL: /* Invalid argument -22 */ 
      write(1,"\nInvalid argument",strlen("\nInvalid argument"));
      break;
      
    case EBADF: /* Bad file number -9 */
      write(1,"\nBad file number",strlen("\nBad file number"));
      break;

    case EACCES: /* Permission denied -13 */
      write(1,"\nPermission denied",strlen("\nPermission denied"));
      break;

    default: /* Undefined error */
      write(1,"\nUndefined error",strlen("\nUndefined error"));
      break;
  }
}
