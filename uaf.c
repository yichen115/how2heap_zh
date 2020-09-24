#include <stdio.h>
#include <stdlib.h>
typedef void (*func_ptr)(char *);
void evil_fuc(char command[])
{
system(command);
}
void echo(char content[])
{
printf("%s",content);
}
int main()
{
    func_ptr *p1=(func_ptr*)malloc(0x20);
    printf("申请了4个int大小的内存");
    printf("p1 的地址: %p\n",p1);
    p1[1]=echo;
    printf("把p1[1]赋值为echo函数，然后打印出\"hello world\"");
    p1[1]("hello world\n");
    printf("free 掉 p1");
    free(p1); 
    printf("因为并没有置为null，所以p1[1]仍然是echo函数，仍然可以输出打印了\"hello again\"");
    p1[1]("hello again\n");
    printf("接下来再去malloc一个p2，会把释放掉的p1给分配出来，可以看到他俩是同一地址的");
    func_ptr *p2=(func_ptr*)malloc(0x20);
    printf("p2 的地址: %p\n",p2);
    printf("p1 的地址: %p\n",p1);
    printf("然后把p2[1]给改成evil_fuc也就是system函数");
    p2[1]=evil_fuc;
    printf("传参调用");
    p1[1]("/bin/sh");
    return 0;
}
