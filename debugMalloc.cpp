#include "debugMalloc.h"

#define malloc malloc
#define realloc realloc
#define free free

void *debugMalloc(size_t size){
	void *ptr=malloc(size);
	bLink->printDebug("malloc'ed %d bytes at 0x%x",size,(unsigned int)ptr);
	return ptr;
}

void *debugRealloc(void *ptr, size_t size){
	unsigned int old_addr=(unsigned int)ptr;
	ptr=realloc(ptr,size);
	bLink->printDebug("realloc'ed %d bytes at 0x%x (org: 0x%x)",size,(unsigned int)ptr, old_addr);
	return ptr;
}

void debugFree(void *ptr){
	bLink->printDebug("freed %d bytes at 0x%x",sizeof(&ptr),(unsigned int)ptr);
	free(ptr);
}



