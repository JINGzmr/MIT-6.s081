#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "syscall.h"
#include "defs.h"

// Fetch the uint64 at addr from the current process.
// 用于从当前进程的地址空间中提取一个 uint64（64位整数）类型的值。
// 它首先获取当前进程的指针 p，然后检查给定的地址 addr 是否有效，即是否在当前进程的地址空间范围内。
// 如果地址有效，它使用 copyin 函数从进程的页表中复制数据到 ip 指向的内存位置，并返回 0 表示成功，否则返回 -1 表示失败。
int
fetchaddr(uint64 addr, uint64 *ip)
{
  struct proc *p = myproc();
  if(addr >= p->sz || addr+sizeof(uint64) > p->sz)
    return -1;
  if(copyin(p->pagetable, (char *)ip, addr, sizeof(*ip)) != 0)
    return -1;
  return 0;
}

// Fetch the nul-terminated string at addr from the current process.
// Returns length of string, not including nul, or -1 for error.
// 用于从当前进程的地址空间中提取一个以空字符（'\0'）结尾的字符串。
// 它首先获取当前进程的指针 p，然后使用 copyinstr 函数从进程的页表中复制字符串到 buf 指向的内存位置，最多复制 max 个字符。
// 如果复制成功，它返回字符串的长度（不包括空字符），如果失败则返回 -1。
int
fetchstr(uint64 addr, char *buf, int max)
{
  struct proc *p = myproc();
  int err = copyinstr(p->pagetable, buf, addr, max);
  if(err < 0)
    return err;
  return strlen(buf);
}

// 这个函数用于从系统调用参数中提取第 n 个参数的原始值。
// 它首先获取当前进程的指针 p，然后根据参数索引 n 获取相应的寄存器值，并返回该值。
static uint64
argraw(int n)
{
  struct proc *p = myproc();
  switch (n) {
  case 0:
    return p->trapframe->a0;
  case 1:
    return p->trapframe->a1;
  case 2:
    return p->trapframe->a2;
  case 3:
    return p->trapframe->a3;
  case 4:
    return p->trapframe->a4;
  case 5:
    return p->trapframe->a5;
  }
  panic("argraw");
  return -1;
}

/*argint、argaddr 和 argstr 函数：
这些函数是对 argraw 函数的包装，用于提取不同类型的系统调用参数。
argint 用于提取整数参数，将结果存储在 ip 指向的地址中。
argaddr 用于提取地址参数，将结果存储在 ip 指向的地址中。
argstr 用于提取字符串参数，将结果存储在 buf 指向的地址中。*/

// Fetch the nth 32-bit system call argument.
int
argint(int n, int *ip)
{
  *ip = argraw(n);
  return 0;
}

// Retrieve an argument as a pointer.
// Doesn't check for legality, since
// copyin/copyout will do that.
int
argaddr(int n, uint64 *ip)
{
  *ip = argraw(n);
  return 0;
}

// Fetch the nth word-sized system call argument as a null-terminated string.
// Copies into buf, at most max.
// Returns string length if OK (including nul), -1 if error.
int
argstr(int n, char *buf, int max)
{
  uint64 addr;
  if(argaddr(n, &addr) < 0)
    return -1;
  return fetchstr(addr, buf, max);
}

extern uint64 sys_chdir(void);
extern uint64 sys_close(void);
extern uint64 sys_dup(void);
extern uint64 sys_exec(void);
extern uint64 sys_exit(void);
extern uint64 sys_fork(void);
extern uint64 sys_fstat(void);
extern uint64 sys_getpid(void);
extern uint64 sys_kill(void);
extern uint64 sys_link(void);
extern uint64 sys_mkdir(void);
extern uint64 sys_mknod(void);
extern uint64 sys_open(void);
extern uint64 sys_pipe(void);
extern uint64 sys_read(void);
extern uint64 sys_sbrk(void);
extern uint64 sys_sleep(void);
extern uint64 sys_unlink(void);
extern uint64 sys_wait(void);
extern uint64 sys_write(void);
extern uint64 sys_uptime(void);
extern uint64 sys_trace(void);

// 这个数组是一个函数指针数组，其中包含了系统调用的具体实现函数。
// 每个系统调用都有一个唯一的编号（例如 SYS_fork、SYS_exit），这些编号被用作索引来查找相应的系统调用函数。
// 每个系统调用函数的地址被存储在数组中的相应索引位置。
static uint64 (*syscalls[])(void) = {
[SYS_fork]    sys_fork,
[SYS_exit]    sys_exit,
[SYS_wait]    sys_wait,
[SYS_pipe]    sys_pipe,
[SYS_read]    sys_read,
[SYS_kill]    sys_kill,
[SYS_exec]    sys_exec,
[SYS_fstat]   sys_fstat,
[SYS_chdir]   sys_chdir,
[SYS_dup]     sys_dup,
[SYS_getpid]  sys_getpid,
[SYS_sbrk]    sys_sbrk,
[SYS_sleep]   sys_sleep,
[SYS_uptime]  sys_uptime,
[SYS_open]    sys_open,
[SYS_write]   sys_write,
[SYS_mknod]   sys_mknod,
[SYS_unlink]  sys_unlink,
[SYS_link]    sys_link,
[SYS_mkdir]   sys_mkdir,
[SYS_close]   sys_close,
[SYS_trace]   sys_trace,
};

// 这个函数是系统调用的入口点。
// 它首先获取当前进程的指针 p，然后从进程的陷阱帧（trap frame）中获取系统调用号（a7 寄存器中的值）。
// 如果系统调用号有效（大于 0 且小于数组 syscalls 的元素个数，并且对应的系统调用函数存在），则调用相应的系统调用函数，并将返回值存储在 a0 寄存器中。
// 如果系统调用号无效，它会打印一条错误消息，然后将 -1 存储在 a0 寄存器中表示调用失败。
void
syscall(void)
{
  int num;
  struct proc *p = myproc();

  num = p->trapframe->a7;
  if(num > 0 && num < NELEM(syscalls) && syscalls[num]) {
    p->trapframe->a0 = syscalls[num]();
  } else {
    printf("%d %s: unknown sys call %d\n",
            p->pid, p->name, num);
    p->trapframe->a0 = -1;
  }
}
