#ifndef _KMALLOC_
#define _KMALLOC_

#include <stddef.h>

#include <utils/types.h>

#define KMALLOC_MINSIZE	16

struct kmalloc_header
{
	unsigned long size : 31;
	unsigned long used : 1;
} __attribute__ ((packed));

void* operator new(size_t size);
void* operator new[](size_t size);

void operator delete(void *mem);
void operator delete[](void *meme);

void* ksbrk(unsigned int);
void* kmalloc(unsigned long);
void* krealloc(void*, unsigned long);
void kfree(const void*);

#endif
