#ifndef UTILS_H
#define UTILS_H

void copy_data(void *start, void *dest, int size);
int copy_from_user(void *start, void *dest, int size);
int copy_to_user(void *start, void *dest, int size);

#define VERIFY_READ	0
#define VERIFY_WRITE	1
int access_ok(int type, const void *addr, unsigned long size);

char is_number(char c);
char is_letter(char c); 
#define min(a,b)	(a<b?a:b)

unsigned long get_ticks(void);

#endif
