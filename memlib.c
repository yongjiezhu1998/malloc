#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define MAX_HEAP (1024 * 1024 * 16)  /* 假设堆最大为16MB */

/*私有的全局变量*/
static char *mem_heap;      /*指向堆的第一个字节*/
static char *mem_brk;       /*指向堆的最后一个字节加一*/
static char *mem_max_addr;  /*最大合法堆地址加一*/

void mem_init(void)
{
    mem_heap = (char *) malloc(MAX_HEAP);
    if (mem_heap == NULL)
    {
        fprintf(stderr, "mem_init_vm: malloc error\n");
	    exit(1);
    }
    mem_brk  = (char *)mem_heap;
    mem_max_addr = (char *) (mem_heap + MAX_HEAP);
}

/*分配器通过调用mem sbrk函数来请求额外的堆存储器，增加incr个字节*/
void *mem_sbrk(int incr)
{
    char *old_brk = mem_brk;

    if (incr < 0 || ((mem_brk + incr) > mem_max_addr))
    {
        errno = ENOMEM;
        fprintf(stderr, "ERROR: mem_sbr failed. Ran out of memory...\n");
        return (void *)-1;
    }

    mem_brk += incr;
    return (void*)old_brk;
}

