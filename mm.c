/*
典型内存布局：
+-------------------+-----------------------+-------------------+
|  Header (WSIZE)   |   Payload (N bytes)   |  Footer (WSIZE)   |
+-------------------+-----------------------+-------------------+
^                   ^                       ^
HDRP(bp)            bp                      FTRP(bp)
*/

/*基本的常量和宏*/
#define WSIZE           4       /*头部和脚部字的大小*/
#define DSIZE           8       /*双字的大小*/
#define CHUNKSIZE       (1<<12) /*初始空闲块的大小和扩展堆时的默认大小*/

#define MAX(x, y)       ((x) > (y) ? (x) : (y))

/*将大小和已分配位结合起来并返回一个值，可以把它存放在头部或者脚部*/
/*/:按位或操作,size后三位预留为0*/
#define PACK            (size, alloc) ((size) | (alloc))

/*读取和返回参数p引用的字*/
#define GET(p)          (*(unsigned int *)(p))  /*将void*强制类型转换为unsigned int *（自然对齐4字节或者8字节），而char* 指向任意地址（无需对齐）*/
#define PUT(p, val)     (*(unsigned int *)(p) = (val))

/*从地址p处的头部或者脚部分别返回大小和已分配位*/
/*0x7=0111, ~取反操作符, &按位与*/
#define GET_SIZE(p)     (GET(p) & ~0x7)
#define GET_ALLOC(p)    (GET(p) & 0x1)

/*分配返回指向这个块的头部和脚部的指针*/
/*char *移动一个字节而unsigned int*则移动4个字节，同时也能按字节精确控制地址*/
#define HDRP(bp)        ((char *)bp - WSIZE)
/*
GET_SIZE(HDRP(bp): 内存块总大小：头部、有效负载和脚部
(char *)bp + GET_SIZE(HDRP(bp)：从有效负载的起始地址 bp 出发，加上块的总大小，理论上指向块末尾的下一个字节。
-DSIZE：脚部和头部WIZE,所以要减去DSIZE
*/
#define FTRP(bp)        ((char *)bp + GET_SIZE(HDRP(bp)) - DSIZE)

/*指向后面块和前面块的指针*/
#define NEXT_BLKP(bp)   ((char *)(bp) + GET_SIZE(HDRP(bp));
/*从前面这个块的脚步获取前面块的size,减去size就是前面块的bp*/
#define PREV_BLKP(bp)   ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)));