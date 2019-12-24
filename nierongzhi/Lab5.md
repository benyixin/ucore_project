# Lab5

这部分实验涉及到用户进程的调度，很明显实验所用代码多出了一个user文件夹。

## lx0

考虑到是用户空间的进程调度，对于一个进程的描述上有一些新的属性，所以除了常规的diff&patch以外还需要对之前的alloc函数进行一些修改。结合新的proc_struct:

```c
struct proc_struct {
    enum proc_state state;                      // Process state
    int pid;                                    // Process ID
    int runs;                                   // the running times of Proces
    uintptr_t kstack;                           // Process kernel stack
    volatile bool need_resched;                 // bool value: need to be rescheduled to release CPU?
    struct proc_struct *parent;                 // the parent process
    struct mm_struct *mm;                       // Process's memory management field
    struct context context;                     // Switch here to run process
    struct trapframe *tf;                       // Trap frame for current interrupt
    uintptr_t cr3;                              // CR3 register: the base addr of Page Directroy Table(PDT)
    uint32_t flags;                             // Process flag
    char name[PROC_NAME_LEN + 1];               // Process name
    list_entry_t list_link;                     // Process link list 
    list_entry_t hash_link;                     // Process hash list
    int exit_code;                              // exit code (be sent to parent proc)
    uint32_t wait_state;                        // waiting state
    struct proc_struct *cptr, *yptr, *optr;     // relations between processes
};
```

需要增加的内容是：

```c
        proc->wait_state = 0;
        proc->cptr = proc->optr = proc->yptr = NULL;
```

//经过查询相关资料，这三个指针的意思分别是子(child)进程，兄(older)进程，弟(younger)进程。

同理，do_forks需要增加：

```c
...
    assert(current->wait_state == 0);
...
	set_links(proc);
...
```

第一句是确保进程处于等待，第二句设置进程链接（即维护进程与其兄弟和父进程的链接）。

在trap.c的idt_init中加入一行

```c
SETGATE(idt[T_SYSCALL], 1, GD_KTEXT, __vectors[T_SYSCALL], DPL_USER);
```

在trap_dispatch加入

```c
current->need_resched = 1;
```

这里主要是将时间片设置为需要调度，配合上面的中断门，会在时间片用完的时候实现对需要调度的进程的区分。



## lx1

这里需要修改do_execv函数，这个函数加载一个内存中的elf程序并执行。

结合注释，这个函数主要执行了六个步骤：

* 为新的进程申请一块mm（用于进行内存管理的数据结构），不成功则报错退出
* 为新的进程申请一片虚拟内存空间，并为其建立页表映射。
* 读取目标ELF文件，在内存中写入其数据段，代码段等信息。 结合ELF文件的结构，这部分需要3.1-3.6这些步骤来实现[1.参考文献：《linux二进制分析》]
* 建立用户栈
* 更新用户进程的虚拟内存空间，并且建立虚拟地址和物理地址的映射关系。
* 为用户态环境设置中断帧，这部分是需要实现的。 

接下来考虑trapframe的结构，tf是一个中断帧指针，总是指向内核栈，在切换到内核态时记录进程中断前的寄存器状态。反之，切换到用户态时取出这些值到寄存器。

```c
struct trapframe {
    struct pushregs tf_regs;
    uint16_t tf_gs;
    uint16_t tf_padding0;
    uint16_t tf_fs;
    uint16_t tf_padding1;
    uint16_t tf_es;
    uint16_t tf_padding2;
    uint16_t tf_ds;
    uint16_t tf_padding3;
    uint32_t tf_trapno;
    /* below here defined by x86 hardware */
    uint32_t tf_err;
    uintptr_t tf_eip;
    uint16_t tf_cs;
    uint16_t tf_padding4;
    uint32_t tf_eflags;
    /* below here only when crossing rings, such as from user to kernel */
    uintptr_t tf_esp;
    uint16_t tf_ss;
    uint16_t tf_padding5;
} __attribute__((packed));
```

结合注释，需要设置的是tf_cs,tf_ds,tf_es,tf_fs,tf_esp,tf_eip,tf_eflags七个值，
```c
	tf->tf_cs = USER_CS;
    tf->tf_ds = tf->tf_es = tf->tf_ss = USER_DS;
    tf->tf_esp = USTACKTOP;
    tf->tf_eip = elf->e_entry;
    tf->tf_eflags = FL_IF;
```

结合下面的定义

```c
#define USER_CS     ((GD_UTEXT) | DPL_USER)
#define USER_DS     ((GD_UDATA) | DPL_USER)
```

上面的代码将tf_cs设置为用户态代码的段选择子（可理解为一个定义为用户态程序TEXT区的宏）

tf_ds等三值为用户态数据区的段选择子，

esp栈顶指针指向栈顶，eip指向elf文件入口地址，最后eflags允许中断。

以上这些全部实现即为一个用户态程序执行的前提。eip会从入口地址依次执行。



## lx2

经过三次调用，do_fork通过copy_range这个函数实现内存资源复制。在copy_range之前只完成了段的创建。

```c
static int
copy_mm(uint32_t clone_flags, struct proc_struct *proc) {
    struct mm_struct *mm, *oldmm = current->mm;

    /* current is a kernel thread */
    if (oldmm == NULL) {
        return 0;
    }
    if (clone_flags & CLONE_VM) {
        mm = oldmm;
        goto good_mm;
    }

    int ret = -E_NO_MEM;
    if ((mm = mm_create()) == NULL) {
        goto bad_mm;
    }
    if (setup_pgdir(mm) != 0) {
        goto bad_pgdir_cleanup_mm;
    }

    lock_mm(oldmm);
    {
        ret = dup_mmap(mm, oldmm);
    }
    unlock_mm(oldmm);

    if (ret != 0) {
        goto bad_dup_cleanup_mmap;
    }

```

//该函数是调用关系中比较重要的一个，用于分析copy_range之前的堆栈情况。

所以需要增加的内容是

```c
void * kva_src = page2kva(page);
        void * kva_dst = page2kva(npage);
    
        memcpy(kva_dst, kva_src, PGSIZE);

        ret = page_insert(to, npage, start, perm);
```

这里的page和npage就是父进程和子进程的虚拟页，将其地址取出后，直接使用memcpy进行内存拷贝，并对子进程的页地址建立与物理地址的对应关系。

### 实现COW

不进行内存的复制，而是将父进程的PDE直接赋值给子进程的PDE，但是需要将其设置为不可写；当子进程需要进行写操作时，再触发中断调用do_pgfault()，给子进程新建PTE，并取代原先PDE中的项，然后才能写入（在新的物理内存上写入）。



## lx3

接下来对fork/exec/wait/exit四个系统调用进行分析：

- fork：在执行了fork系统调用之后，会执行正常的中断处理流程，之后根据系统调用号执行sys_fork函数，再调用中的do_fork函数，完成新的进程的进程控制块的初始化、设置、以及将父进程内存中的内容到子进程的内存的复制工作，然后将新创建的进程放入可执行队列（runnable）；它不会影响当前进程的运行状态。
- exec：在执行了exec系统调用之后，会执行正常的中断处理流程，之后根据系统调用号执行sys_exec函数，调用do_execve函数，在该函数中，会对内存空间先初始化再将新的要执行的程序加载到内存中，然后设置好mm，调用load_icode来使之能执行。也不会影响当前进程的运行状态。
- wait：用于等待子进程结束。根据pid找一个处于退出状态的子进程，如果所找到的状态是zombie，则释放资源然后返回。如果不为zombie则将当前进程休眠（sleeping）,等待子进程exit并将其唤醒，这时改变了当前进程的运行状态。
- exit：如果当前进程是一个用户进程，则释放mm结构、页目录表占用的内存等，将自己的状态设为zombie，并唤醒父进程，调用schedule，等待其回收资源。

生命周期图大致可以表示为：

![QQ截图20191210135903](D:\课件2019\操作系统\pic\QQ截图20191210135903.png)

