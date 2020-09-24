#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    fprintf(stderr, "尽管这个例子没有演示攻击效果，但是它演示了 glibc 的分配机制\n");
    fprintf(stderr, "glibc 使用首次适应算法选择空闲的堆块\n");
    fprintf(stderr, "如果有一个空闲堆块且足够大，那么 malloc 将选择它\n");
    fprintf(stderr, "如果存在 use-after-free 的情况那可以利用这一特性\n");

    fprintf(stderr, "首先申请两个比较大的 chunk\n");
    char* a = malloc(0x512);
    char* b = malloc(0x256);
    char* c;

    fprintf(stderr, "第一个 a = malloc(0x512) 在: %p\n", a);
    fprintf(stderr, "第二个 a = malloc(0x256) 在: %p\n", b);
    fprintf(stderr, "我们可以继续分配\n");
    fprintf(stderr, "现在我们把 \"AAAAAAAA\" 这个字符串写到 a 那里 \n");
    strcpy(a, "AAAAAAAA");
    fprintf(stderr, "第一次申请的 %p 指向 %s\n", a, a);

    fprintf(stderr, "接下来 free 掉第一个...\n");
    free(a);

    fprintf(stderr, "接下来只要我们申请一块小于 0x512 的 chunk，那就会分配到原本 a 那里: %p\n", a);

    c = malloc(0x500);
    fprintf(stderr, "第三次 c = malloc(0x500) 在: %p\n", c);
    fprintf(stderr, "我们这次往里写一串 \"CCCCCCCC\" 到刚申请的 c 中\n");
    strcpy(c, "CCCCCCCC");
    fprintf(stderr, "第三次申请的 c %p 指向 %s\n", c, c);
    fprintf(stderr, "第一次申请的 a %p 指向 %s\n", a, a);
    fprintf(stderr, "可以看到，虽然我们刚刚看的是 a 的，但它的内容却是 \"CCCCCCCC\"\n");
}
