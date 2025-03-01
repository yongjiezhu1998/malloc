#include <memlib.h>
#include <stddef.h>
#include <mm.h>

/*创建一个带初始化空闲块的堆*/
int mm_init(void)
{
    void *heap_listp;
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void*)-1)
    {
        return -1;
    }

    /*堆起始位置填充一个字用于双字对齐*/
    PUT(heap_listp, 0);     
    
    /*序言块，一头一尾*/
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1));  /* Prologue header */
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1));  /* Prologue footer */

    /*结尾块，大小为0的已分配块，只由一个头部组成*/
    PUT(heap_listp + (3*WSIZE), PACK(0, 1));     /* Epilogue header */

    heap_listp += (2*WSIZE);
}

static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;


    // 将请求大小向上舍入为最接近2字（8字节）的倍数
    size = (words % 2) ? (words+1)*WSIZE : words*WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
    {
        return NULL;
    }

    /*将新内存块的头部初始化为大小为size，空闲标志0*/
    PUT(HDRP(bp), PACK(size, 0));
    /*设置脚部*/
    PUT(FTRP(bp), PACK(size, 0));
    /*设置新的堆结尾*/
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1));    /* New epilogue header */

    return coalesce(bp);
}

/*释放一个块*/
void mm_free(void *bp)
{
    /*释放一个块，并使用边界标记合并将其与所有相邻空闲块在常数时间内合并*/
    size_t size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 0));
    PUT(HDRP(bp), PACK(size, 0));
    colalesce(bp);
}

static void* coalesce(void *bp)
{
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    /*Case1: 前面的块和后面的块都是已分配的*/
    if(prev_alloc && next_alloc)
    {
        return bp;
    }

    /*Case2: 前面的块是已分配的，后面的块是空闲的*/
    else if(prev_alloc && !next_alloc)
    {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(szie, 0));
    }

    /*Case3: 前面的块是空闲的，后面的块是已分配的*/
    else if(!prev_alloc && next_alloc)
    {
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(FTRP(bp), PACK(szie, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }

    /*Case4: 前面的和后面的块都是空闲的*/
    else
    {
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
        GET_SIZE(FTRP(NEXT_BLKP(bp)));

        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(HDRP(NEXT_BLKP(bp)), PACK(size, 0));
        bp = PREV_BLKP(bp);
    }
    return bp;
}

/*从空闲链表分配一个块*/
void *mm_malloc(size_t size)
{
    size_t asize;
    size_t extendsize;
    char *bp;

    if (size == 0)
    {
        return NULL;
    }

    /*满足字对齐要求*/
    if (size < DSIZE)
    {
        asize = 2*DSIZE;
    }
    else
    {   
        /*
        对齐操作：

            size + DSIZE：用户数据 size 加上头部大小 DSIZE，得到总需求空间。

            + (DSIZE - 1)：通过添加 DSIZE-1，确保后续除法实现向上取整。

            / DSIZE：用整数除法截断多余部分，得到对齐后的块数。

            * DSIZE：将块数转换为实际字节数，确保最终大小是 DSIZE 的整数倍。
        */
        asize = DSIZE * ((size + (DSIZE) + (DSIZE -1 )) / DSIZE);
    }

    /*找到空闲块*/
    if ((bp = find_fit(asize)) != NULL)
    {
        /*放置请求块*/
        place(bp, size);
        /*请求块*/
        return bp;
    }

    /*不能发现匹配的块，就用一个新的空闲块来扩展堆*/
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extend_heap/WSIZE)) == NULL)
    {
        return NULL;
    }

    place(bp, size);
    return bp;
}

static void *find_fit(size_t asize)
{
    void *bp;

    for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp))
    {
        if (!GET_ALLOC(HDRP(bp)) && (asize <= GET_SIZE(bp)))
        {
            return bp;
        }
    }

    return NULL:
}

static void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));

    if ((csize - asize) > 2*DSIZE)
    {
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize - asize, 1));
        PUT(FTRP(bp), PACK(csize - asize, 1));
    }
    else
    {
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
    }