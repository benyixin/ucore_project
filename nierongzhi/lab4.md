# Lab4

这部分实验主要是进程的创建，执行，切换和调度。

## lx1

首先分析``proc_struct``这个描述一个进程的结构体，结合注释易得以下结论：

```c
struct proc_struct {
    enum proc_state state;                      // Process state，该值在未初始化或未开始的情况下则为0，休眠则为1，运行则为2，僵尸进程则为3.
    int pid;                                    // Process ID，进程id
    int runs;                                   // the running times of Proces，进程创建后经过的时间
    uintptr_t kstack;                           // Process kernel stack，内核栈（地址）
    volatile bool need_resched;                 // bool value: need to be 是否需要调度rescheduled to release CPU?
    struct proc_struct *parent;                 // the parent process父进程
    struct mm_struct *mm;                       // Process's memory management field，虚拟内存
    struct context context;                     // Switch here to run process上下文（这个在后面详细说）
    struct trapframe *tf;                       // Trap frame for current interrupt，中断帧指针
    uintptr_t cr3;                              // CR3 register: the base addr of Page Directroy Table(PDT)当前页表地址
    uint32_t flags;                             // Process flag，标志，具体什么用途尚不清楚
    char name[PROC_NAME_LEN + 1];               // Process name，进程名
    list_entry_t list_link;                     // Process link list 进程链表
    list_entry_t hash_link;                     // Process hash list每个进程对应了一个哈希值，将哈希值构造成链表。
};
```



##### struct context context

是进程上下文，用于辅助进程在内核态的切换。在进程切换时，保存前一个进程的寄存器状态，使进程切换后可以正常恢复到原来的工作现场。

##### struct trapframe tf*

中断帧指针，总是指向内核栈的某个位置，当进程从用户空间跳到内核空间时，中断帧记录了进程在被中断前的状态（保存在栈里），所以跳回用户空间时，需要访问栈内存来读取之前的寄存器状态。

实现的初始化函数如下：

```c
static struct proc_struct *
alloc_proc(void) {
    struct proc_struct *proc = kmalloc(sizeof(struct proc_struct));
    if (proc != NULL) {
    //LAB4:EXERCISE1 YOUR CODE
    /*
     * below fields in proc_struct need to be initialized
     *       enum proc_state state;                      // Process state
     *       int pid;                                    // Process ID
     *       int runs;                                   // the running times of Proces
     *       uintptr_t kstack;                           // Process kernel stack
     *       volatile bool need_resched;                 // bool value: need to be rescheduled to release CPU?
     *       struct proc_struct *parent;                 // the parent process
     *       struct mm_struct *mm;                       // Process's memory management field
     *       struct context context;                     // Switch here to run process
     *       struct trapframe *tf;                       // Trap frame for current interrupt
     *       uintptr_t cr3;                              // CR3 register: the base addr of Page Directroy Table(PDT)
     *       uint32_t flags;                             // Process flag
     *       char name[PROC_NAME_LEN + 1];               // Process name
     */
        proc->state = PROC_UNINIT;
        proc->pid = -1;
        proc->runs = 0;
        proc->kstack = 0;
        proc->need_resched = 0;
        proc->parent = NULL;
        proc->mm = NULL;
        memset(&(proc->context), 0, sizeof(struct context));
        proc->tf = NULL;
        proc->cr3 = boot_cr3;
        proc->flags = 0;
        memset(proc->name, 0, PROC_NAME_LEN);
    }
    return proc;
}
```

这里将进程的状态设为未开始，pid设为-1（有专门的机制来设这个值，具体在lx2），经过时间0，内核栈地址为0（还没有被调度，置空），不需调度，父进程不存在，页表不存在，上下文设为全0，中断帧指针为空（不存在），cr3值为*内核页目录表的基址* ，标志位，进程名均为0.

## lx2

基于对已有的代码的分析，一个进程调用过程中的资源分配需要经过以下步骤：

* 调用alloc_proc，首先获得一块用户信息块。 
* 为进程分配一个内核栈。
*  复制原进程的内存管理信息到新进程（但内核线程不必做此事）
*  复制原进程上下文到新进程 
* 将新进程添加到进程列表 
* 唤醒新进程
* 返回新进程号

七个步骤的实现在下面用注释标出了：

```c
int
do_fork(uint32_t clone_flags, uintptr_t stack, struct trapframe *tf) {
    int ret = -E_NO_FREE_PROC;
    struct proc_struct *proc;
    if (nr_process >= MAX_PROCESS) {
        goto fork_out;
    }
    ret = -E_NO_MEM;
    //LAB4:EXERCISE2 YOUR CODE
    /*
     * Some Useful MACROs, Functions and DEFINEs, you can use them in below implementation.
     * MACROs or Functions:
     *   alloc_proc:   create a proc struct and init fields (lab4:exercise1)
     *   setup_kstack: alloc pages with size KSTACKPAGE as process kernel stack
     *   copy_mm:      process "proc" duplicate OR share process "current"'s mm according clone_flags
     *                 if clone_flags & CLONE_VM, then "share" ; else "duplicate"
     *   copy_thread:  setup the trapframe on the  process's kernel stack top and
     *                 setup the kernel entry point and stack of process
     *   hash_proc:    add proc into proc hash_list
     *   get_pid:      alloc a unique pid for process
     *   wakeup_proc:  set proc->state = PROC_RUNNABLE
     * VARIABLES:
     *   proc_list:    the process set's list
     *   nr_process:   the number of process set
     */

    //    1. call alloc_proc to allocate a proc_struct
    //    2. call setup_kstack to allocate a kernel stack for child process
    //    3. call copy_mm to dup OR share mm according clone_flag
    //    4. call copy_thread to setup tf & context in proc_struct
    //    5. insert proc_struct into hash_list && proc_list
    //    6. call wakeup_proc to make the new child process RUNNABLE
    //    7. set ret vaule using child proc's pid
    if ((proc = alloc_proc()) == NULL) {
        return ret;
    }
	//1
    proc->parent = current;

    if (setup_kstack(proc) != 0) {
        put_kstack(proc);
        kfree(proc);
        return ret;
    }//2
    if (copy_mm(clone_flags, proc) != 0) {
        kfree(proc);
        return ret;
    }//3
    copy_thread(proc, stack, tf);//4

    bool intr_flag;
    local_intr_save(intr_flag);
    {
        proc->pid = get_pid();
        hash_proc(proc);
        list_add(&proc_list, &(proc->list_link));//5
        nr_process ++;
    }
    local_intr_restore(intr_flag);

    wakeup_proc(proc);//6

    ret = proc->pid;//7

    return ret;
}
```



## lx3

```c
void
proc_run(struct proc_struct *proc) {
    if (proc != current) {
        bool intr_flag;
        struct proc_struct *prev = current, *next = proc;
        local_intr_save(intr_flag);
        {
            current = proc;
            load_esp0(next->kstack + KSTACKSIZE);
            lcr3(next->cr3);
            switch_to(&(prev->context), &(next->context));
        }
        local_intr_restore(intr_flag);
    }
}
```

以上是proc_run的代码；

intr_flag是中断变量，首先屏蔽中断，然后在大括号内的代码里修改当前进程为新进程，修改页表项，再进行上下文切换。最后允许中断，这个机制是典型的加解锁，这个可以防止进程切换过程中遇到中断（回答问题2）。

所以一共存在两个进程：proc（新进程）和调用proc的进程。

最后输出：

![QQ截图20191210140749](D:\课件2019\操作系统\pic\QQ截图20191210140749.png)