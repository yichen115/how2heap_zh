#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

int main(int argc , char* argv[]){

    intptr_t *p1,*p2,*p3,*p4;
    fprintf(stderr, "这是一个简单的堆块重叠问题，首先申请 3 个 chunk\n");

    p1 = malloc(0x100 - 8);
    p2 = malloc(0x100 - 8);
    p3 = malloc(0x80 - 8);
    fprintf(stderr, "这三个 chunk 分别申请到了:\np1：%p\np2：%p\np3：%p\n给他们分别填充\"1\"\"2\"\"3\"\n\n", p1, p2, p3);

    memset(p1, '1', 0x100 - 8);
    memset(p2, '2', 0x100 - 8);
    memset(p3, '3', 0x80 - 8);

    fprintf(stderr, "free 掉 p2\n");
    free(p2);
    fprintf(stderr, "p2 被放到 unsorted bin 中\n");

    fprintf(stderr, "现在假设有一个堆溢出漏洞，可以覆盖 p2\n");
    fprintf(stderr, "为了保证堆块稳定性，我们至少需要让 prev_inuse 为 1，确保 p1 不会被认为是空闲的堆块\n");

    int evil_chunk_size = 0x181;
    int evil_region_size = 0x180 - 8;
    fprintf(stderr, "我们将 p2 的大小设置为 %d, 这样的话我们就能用 %d 大小的空间\n",evil_chunk_size, evil_region_size);

    *(p2-1) = evil_chunk_size; // 覆盖 p2 的 size

    fprintf(stderr, "\n现在让我们分配另一个块，其大小等于块p2注入大小的数据大小\n");
    fprintf(stderr, "malloc 将会把前面 free 的 p2 分配给我们（p2 的 size 已经被改掉了）\n");
    p4 = malloc(evil_region_size);

    fprintf(stderr, "\np4 分配在 %p 到 %p 这一区域\n", (char *)p4, (char *)p4+evil_region_size);
    fprintf(stderr, "p3 从 %p 到 %p\n", (char *)p3, (char *)p3+0x80-8);
    fprintf(stderr, "p4 应该与 p3 重叠，在这种情况下 p4 包括所有 p3\n");

    fprintf(stderr, "这时候通过编辑 p4 就可以修改 p3 的内容，修改 p3 也可以修改 p4 的内容\n\n");

    fprintf(stderr, "接下来验证一下，现在 p3 与 p4:\n");
    fprintf(stderr, "p4 = %s\n", (char *)p4+0x10);
    fprintf(stderr, "p3 = %s\n", (char *)p3+0x10);

    fprintf(stderr, "\n如果我们使用 memset(p4, '4', %d), 将会:\n", evil_region_size);
    memset(p4, '4', evil_region_size);
    fprintf(stderr, "p4 = %s\n", (char *)p4+0x10);
    fprintf(stderr, "p3 = %s\n", (char *)p3+0x10);

    fprintf(stderr, "\n那么之后再 memset(p3, '3', 80), 将会:\n");
    memset(p3, '3', 80);
    fprintf(stderr, "p4 = %s\n", (char *)p4+0x10);
    fprintf(stderr, "p3 = %s\n", (char *)p3+0x10);
}
