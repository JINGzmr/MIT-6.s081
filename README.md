# 学习笔记notes

## 引言

- 操作系统满足的条件：多路复用、隔离、交互
- Xv6运行在多核RISC-V微处理器上
  - RISC-V是一个64位的中央处理器
  - 基于“LP64”的C语言编写
  - `long`（L）和指针（P）变量是64位，但`int`是32位

---

## 抽象系统资源

- Unix应用程序只通过文件系统的`open`、`read`、`write`和`close`系统调用与存储交互，而**不是直接读写磁盘**
- Unix进程使用`exec`来构建它们的内存映像，而**不是直接与物理内存交互**

---

## 用户态，核心态，以及系统调用

#### 用户态=用户模式=目态

#### 核心态=管理模式=管态

- CPU为强隔离提供硬件支持

- RISC-V有三种CPU可以执行指令的模式：**机器模式**(Machine Mode)、**用户模式**(User Mode)和**管理模式**(Supervisor Mode)

- 机器模式：
  - 执行的指令具有完全特权
  - CPU在机器模式下启动
  - 主要用于配置计算机
  - 执行部分代码后，进入管理模式
  
- 管理模式：
  - CPU被允许执行**特权指令**，在**内核空间**中运行
    - 启用和禁用中断、读取和写入保存页表地址的寄存器
  - **内核**：在此模式下（或内核空间）中运行的软件
  
- 用户模式：
  - 程序**无法执行特权指令**，而是要切换到管理模式
    - 所以想要调用内核函数（像read）的应用程序，必须过度到内核
    - `ecall`指令：将CPU从用户模式切换到管理模式，并在**内核指定的入口点**进入内核--->入口点绝对是内核决定的
    - 内核对应用程序请求的操作有**决定权**
  - 应用程序执行用户模式的指令，在**用户空间**中运行
  
  
  
  ### 用户空间与内核空间
  
- 用户空间运行的程序运行在user mode，内核空间的程序运行在kernel mode

- 操作系统位于内核空间

- ![image-20231206212822221](/home/zhangminrui/.config/Typora/typora-user-images/image-20231206212822221.png)

---

## 内核组织

- **宏内核**（monolithic kernel）：整个操作系统都驻留在内核中，所有系统调用的实现都以管理模式运行
  - 优点：整个操作系统以完全的硬件特权运行；操作系统的不同部分更容易合作
  - 缺点：不同部分之间的接口通常很复杂，管理模式中的错误经常会导致内核失败
  - 解决办法：减少在管理模式下运行的操作系统代码量，并**在用户模式下执行大部分操作系统 **---> **微内核**（microkernel）

![image-20231123160525729](/home/zhangminrui/.config/Typora/typora-user-images/image-20231123160525729.png)

- 文件系统作为用户级进程运行

- 服务器：作为**进程运行**的**操作系统服务**

  > 客户/服务器（Client/Server）模式:
  >
  > 单机微内核操作系统都采用客户/服务器模式，将操作系统中最基本的部分放入内核中，而**把操作系统的绝大部分功能都放在微内核外面的一组服务器([进程](https://baike.baidu.com/item/进程))中实现**

- 为了允许应用程序与文件服务器交互，内核提供了允许从一个用户态进程向另一个用户态进程发送消息的进程间通信机制。例如，如果像shell这样的应用程序想要读取或写入文件，它会向文件服务器发送消息并等待响应。

- xv6属于宏内核

  - 因此，xv6内核接口对应于操作系统接口，内核实现了完整的操作系统

---

## xv6架构

- XV6的源代码位于**kernel /**子目录中

- 源代码按照模块化的概念划分为多个文件

- 模块间的接口都被定义在了**def.h**（**kernel/defs.h**）

  

- | **文件**          | **描述**                                    |
  | ----------------- | :------------------------------------------ |
  | **bio.c**         | 文件系统的磁盘块缓存                        |
  | **console.c**     | 连接到用户的键盘和屏幕                      |
  | **entry.S**       | 首次启动指令                                |
  | **exec.c**        | `exec()`系统调用                            |
  | **file.c**        | 文件描述符支持                              |
  | **fs.c**          | 文件系统                                    |
  | **kalloc.c**      | 物理页面分配器                              |
  | **kernelvec.S**   | 处理来自内核的陷入指令以及计时器中断        |
  | **log.c**         | 文件系统日志记录以及崩溃修复                |
  | **main.c**        | 在启动过程中控制其他模块初始化              |
  | **pipe.c**        | 管道                                        |
  | **plic.c**        | RISC-V中断控制器                            |
  | **printf.c**      | 格式化输出到控制台                          |
  | **proc.c**        | 进程和调度                                  |
  | **sleeplock.c**   | Locks that yield the CPU                    |
  | **spinlock.c**    | Locks that don’t yield the CPU.             |
  | **start.c**       | 早期机器模式启动代码                        |
  | **string.c**      | 字符串和字节数组库                          |
  | **swtch.c**       | 线程切换                                    |
  | **syscall.c**     | Dispatch system calls to handling function. |
  | **sysfile.c**     | 文件相关的系统调用                          |
  | **sysproc.c**     | 进程相关的系统调用                          |
  | **trampoline.S**  | 用于在用户和内核之间切换的汇编代码          |
  | **trap.c**        | 对陷入指令和中断进行处理并返回的C代码       |
  | **uart.c**        | 串口控制台设备驱动程序                      |
  | **virtio_disk.c** | 磁盘设备驱动程序                            |
  | **vm.c**          | 管理页表和地址空间                          |

  > 注：`syscall.c`：操作系统内核中用于处理系统调用的**核心部分**，它负责**从用户态切换到内核态**，根据系统调用号来**执行相应的系统调用函数**，并将结果返回给用户程序。这是操作系统的关键功能之一，用于**实现用户程序与硬件之间的交互**。

  > 注：`proc.c`：多进程操作系统内核的进程管理机制，允许**多个进程在共享计算资源的情况下并发执行**，用于**创建、管理和调度进程**，以及提供了一些与进程相关的功能，如进程创建、进程退出、等待子进程、唤醒进程等。以**实现多任务和多用户的支持**
  >
  > > 1. 进程初始化 (`procinit` 和 `proc_pagetable` 函数)：在系统启动时**初始化进程表和进程页表，为每个进程分配内核栈和页表**。`procinit` 函数初始化进程表和内核栈，而 `proc_pagetable` 函数为进程创建用户页表，并映射 trampoline 代码和 trapframe。
  > > 2. 进程创建 (`allocproc` 和 `fork` 函数)：`allocproc` 函数用于**分配一个新的进程控制块**（`struct proc`）以及相关资源。`fork` 函数复制父进程的内存空间和上下文。
  > > 3. 进程退出 (`exit` 函数)：`exit` 函数用于**终止当前进程的执行，关闭该进程打开的文件，释放进程占用的资源，并将进程状态设置为 ZOMBIE**。
  > > 4. 进程等待 (`wait` 函数)：如果没有子进程退出，父进程将会被阻塞，直到有子进程退出。
  > > 5. 进程调度 (`scheduler` 函数)：`scheduler` 函数是一个无限循环，**负责选择要运行的进程，并在不同进程之间切换**。它会扫描进程表，**找到处于可运行状态的进程，并进行上下文切换，将 CPU 分配给选定的进程。**
  > > 6. 进程睡眠和唤醒 (`sleep` 和 `wakeup` 函数)

---

## 进程概述

- Xv6中的隔离单位是一个进程

- **进程抽象**防止一个进程破坏或监视另一个进程的内存、CPU、文件描述符等。它还防止一个进程破坏内核本身

- 内核用来**实现进程的机制**包括：用户/管理模式标志、地址空间和线程的时间切片

- Xv6使用**页表**（由硬件实现）为每个进程提供自己的地址空间

  > RISC-V页表作用：**将虚拟地址（RISC-V指令操纵的地址）转换（或“映射”）为物理地址（CPU芯片发送到主存储器的地址）**

![image-20231124212931101](/home/zhangminrui/.config/Typora/typora-user-images/image-20231124212931101.png)

> 操作系统可能会使用不同的虚拟地址空间部分来隔离栈和堆，或者出于安全性考虑，操作系统**可能会将栈映射到较低的虚拟地址**，以减少栈溢出对其他部分的潜在影响
>
> (xv6和我们的x86可能就是不太一样)

- Xv6**为每个进程维护一个单独的页表**，定义了该进程的地址空间

- 硬件在页表中查找虚拟地址时只使用低39位；xv6只使用这39位中的38位。因此，最大地址是2^38-1=0x3fffffffff，即`MAXVA`（定义在**kernel/riscv.h**:348）

- xv6为`trampoline`（用于在用户和内核之间切换）和`trapframe`（用于映射进程切换到内核）分别保留了一个页面

  

- xv6内核**为每个进程维护许多状态片段**，并将它们聚集到一个`proc`(**kernel/proc.h**:86)结构体中

  - **一个进程最重要的内核状态片段是它的页表、内核栈区和运行状态**
  - 使用符号`p->xxx`来引用`proc`结构体的元素
    - `p->pagetable`：指向该进程页表的指针
    - `p->kstack`：内核栈区
    - `p->state`：表明进程是已分配、就绪态、运行态、等待I/O中（阻塞态）还是退出



- **每个进程都有一个执行线程（或简称线程）来执行进程的指令**
  - 在进程之间切换：内核挂起当前运行的线程，并恢复另一个进程的线程
  - 线程的大部分状态存储在线程的栈区上
  - 线程可以在内核中“阻塞”等待I/O，并在I/O完成后恢复到中断的位置
- **每个进程有两个栈区：一个用户栈区和一个内核栈区（`p->kstack`）**
  - 用户栈：进程执行用户指令时
  - 内核栈：进程进入内核（由于系统调用或中断）时，内核代码在上面执行

> 一个进程可以通过执行RISC-V的`ecall`指令进行系统调用，该指令提升硬件特权级别，并将程序计数器（PC）更改为内核定义的入口点，入口点的代码切换到内核栈，执行实现系统调用的内核指令，当系统调用完成时，内核切换回用户栈，并通过调用`sret`指令返回用户空间，该指令降低了硬件特权级别，并在系统调用指令刚结束时恢复执行用户指令

---

## 启动xv6和第一个进程

1. RISC-V计算机上电时,它会**初始化自己**并**运行**一个存储在只读内存中的**引导加载程序**

2. 引导加载程序**将xv6内核加载到内存中**

   > 此时页式硬件被禁用
   >
   > 因此虚拟地址将直接映射到物理地址

   > 加载到物理地址为`0x80000000`的内存中
   >
   > 因为地址范围`0x0 ~ 0x80000000`包含I/O设备，所以从`0x80000000`开始

3. **机器模式下**，中央处理器从`_entry` (**kernel/entry.S**:6)开始运行xv6

4. `_entry`的指令**设置了一个栈区**，这样xv6就可以**运行C代码**

   > Xv6在**start. c (kernel/start.c:11)**文件中为初始栈***stack0\***声明了空间
   >
   > 栈是向下扩展的，所以`_entry`的代码将栈顶地址`stack0+4096`加载到栈顶指针寄存器`sp`中
   >
   > 此时内核有了栈区，`_entry`便调用C代码`start`(**kernel/start.c**:21)

5. 函数`start`在机器模式下执行部分操作，然后**切换到管理模式**

   > 其中一个操作是：对时钟芯片进行编程以产生计时器中断
   >
   > 指令`mret`：进入管理模式
   >
   > ~~但这里是用到其他的方式~~

6. 在`main`(**kernel/main.c**:11)初始化几个设备和子系统后，便通过调用`userinit` (**kernel/proc.c**:212)**创建第一个进程**

   > 第一个进程执行一个小型程序：**initcode. S** (**user/initcode.S:**1)
   >
   > 它通过调用`exec`系统调用**重新进入内核**
   >
   > > `exec`用一个新程序（本例中为 `/init`）替换当前进程的内存和寄存器

7. 当内核完成`exec`，该小型程序就**返回**`/init`进程中的**用户空间**

   > 有时候，`init`(***user/init.c\***:15)将创建一个新的控制台设备文件，然后以文件描述符0、1和2打开它
   >
   > 然后它**在控制台上启动一个shell**

8. 系统启动完毕

---

2023.11.28





