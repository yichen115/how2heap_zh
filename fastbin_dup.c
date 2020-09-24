#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main()
{
    fprintf(stderr, "这个例子演示了 fastbin 的 double free\n");

    fprintf(stderr, "首先申请了 3 个 chunk\n");
    char* a = malloc(8);
    strcpy(a, "AAAAAAAA");
    char* b = malloc(8);
    strcpy(b, "BBBBBBBB");
    char* c = malloc(8);
    strcpy(c, "CCCCCCCC");

    fprintf(stderr, "第一个 malloc(8): %p\n", a);
    fprintf(stderr, "第二个 malloc(8): %p\n", b);
    fprintf(stderr, "第三个 malloc(8): %p\n", c);

    fprintf(stderr, "free 掉第一个\n");
    free(a);

    fprintf(stderr, "当我们再次 free %p 的时候, 程序将会崩溃因为 %p 在 free 链表的第一个位置上\n", a, a);
    // free(a);
    fprintf(stderr, "我们先 free %p.\n", b);
    free(b);

    fprintf(stderr, "现在我们就可以再次 free %p 了, 因为他现在不在 free 链表的第一个位置上\n", a);
    free(a);
    fprintf(stderr, "现在空闲链表是这样的 [ %p, %p, %p ]. 如果我们 malloc 三次, 我们会得到两次 %p \n", a, b, a, a);
    
    char* d = malloc(8);
    char* e = malloc(8);
    char* f = malloc(8);
    strcpy(d, "DDDDDDDD");
    strcpy(e, "EEEEEEEE");
    strcpy(f, "FFFFFFFF");
    fprintf(stderr, "第一次 malloc(8): %p\n", d);
    fprintf(stderr, "第二次 malloc(8): %p\n", e);
    fprintf(stderr, "第三次 malloc(8): %p\n", f);
}
