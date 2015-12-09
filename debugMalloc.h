#ifndef DEBUGMALLOC_H
#define DEBUGMALLOC_H
#include "Brewpi.h"

#define malloc debugMalloc
#define realloc debugRealloc
#define free debugFree

void *debugMalloc(size_t size);
void *debugRealloc(void *ptr, size_t size);

void debugFree(void *ptr);


#endif
