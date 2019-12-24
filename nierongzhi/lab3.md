# lab3

这个实验主要是实现页面映射和页表的页替换算法

## lx1

分析这个do_pgfault函数，根据注释这个函数需要完成：

* 检查页面异常错误码，其最后两位是存在位，读/写位。如果发现错误则打印错误信息。这部分在源代码317-334行已经有了（&3是跟00000011这个数做按位与，只保留error code的后两位）
* 对于没有错误的情况，如果addr索引到的页表项为0，那么这是一个未分配物理页的情况，我们需要调用get_pte来申请分配一个物理页，然后设置到物理页的映射（pgdir_alloc_page）



//对于下一个练习而言 还需要设置页面为swappable。

```c
if ((ptep = get_pte(mm->pgdir, addr, 1)) == NULL) {
        cprintf("get_pte in do_pgfault failed\n");
        goto failed;
    }
    
    if (*ptep == 0) { // if the phy addr isn't exist, then alloc a page & map the phy addr with logical addr
        if (pgdir_alloc_page(mm->pgdir, addr, perm) == NULL) {
            cprintf("pgdir_alloc_page in do_pgfault failed\n");
            goto failed;
        }
    }
```

这两个if的作用是：首先申请分配物理页，同时确认这次分配是否成功。若不成功（为NULL）则打印错误信息，如果ptep（指向页表项的指针）值为空（该页表项不存在），则调用pgdir_alloc_page（一个分配页大小和设置地址映射的函数）来建立到物理页的映射，如果也不成功则打印错误信息。

![QQ截图20191129194927](D:\课件2019\操作系统\pic\QQ截图20191129194927.png)

这种情况下已经能够打印出check_pgfault()，说明练习1的目的已经完成，但是后面的报错也说明页表的分配和交换功能是有误的（在练习2中完善）



## lx2

要实现页表的交换，也即在ptep不为0时（表示对应页表项被换到磁盘中），也要新建一个物理页，再根据addr来把换出的数据写到新建的物理页上。接下来要设置页表为可交换，把它插入到可置换物理页链表的末尾。

```c
 if(swap_init_ok) {
            struct Page *page=NULL;
            if ((ret = swap_in(mm, addr, &page)) != 0) {
                cprintf("swap_in in do_pgfault failed\n");
                goto failed;
            }    
            page_insert(mm->pgdir, page, addr, perm);
            swap_map_swappable(mm, addr, page, 1);
            page->pra_vaddr = addr;
        }
        else {
            cprintf("no swap_init_ok but ptep is %x, failed\n",*ptep);
            goto failed;
        }
```

mm_struct维护了一个双向链表，每个节点存储一组属于同一个页表的虚拟内存空间。其中的成员pgdir指向的是对应的页表。perm是状态位。将新建的物理页插入到链表末端。然后把页面设为可替换，再将其地址（addr，虚拟地址）赋值给新建的页。

然后还要对swap_out_victim进行分析。由于我们是一个先进先出算法（对应队列），而新的节点是从链表尾部增加进去的，那么头部即是最早插入的节点，在需要页面替换时会优先被换出。



```c
	 list_entry_t *le = head->prev;
     assert(head!=le);
     struct Page *p = le2page(le, pra_page_link);
     list_del(le);
     assert(p !=NULL);
     *ptr_page = p;
```

list_entry_t在之前的作业中已经分析过，是指内存分配中的链表的一个入口（指针），那么le通常即指向要换出的页表。找到其对应的页面并保存其copy（通过ptr_page传参传回，数据准备放进硬盘），le对应节点被删除。

再分析_fifo_map_swappable函数，这个函数的功能是将最新近的节点页面加到链表的末端，已有的代码已经将head指针赋值为 sm_priv（一个指针，指向用来链接记录页访问情况的链表头，这里即为最不常使用的链表的一端，考虑到list_entry_t是环形的，故队尾和队首逻辑相邻，指的是队尾队首的交接处），entry指向要插入的页面。我们只需要加一句：

```c
list_add(head, entry);
```

调用已有的函数将其插入即可。

运行效果如图

![QQ截图20191129163225](D:\课件2019\操作系统\pic\QQ截图20191129163225.png)

## 解答相关问题

1.页目录项和页表都是32位，架构如下：

>
>
>前20位表示4K对齐的该PDE对应的页表起始位置（物理地址，该物理地址的高20位即PDE中的高20位，低12位为0）；
>
>PDE:
>
>- 第9-11位未被CPU使用，可保留给OS使用；
>- 接下来的第8位可忽略；
>- 第7位用于设置Page大小，0表示4KB；
>- 第6位恒为0；
>- 第5位用于表示该页是否被使用过；
>- 第4位设置为1则表示不对该页进行缓存；
>- 第3位设置是否使用write through缓存写策略；
>- 第2位表示该页的访问需要的特权级；
>- 第1位表示是否允许读写；
>- 第0位为该PDE的存在位；
>
>PTE:
>
>* 高20位与PDE相似的，用于表示该PTE指向的物理页的物理地址；
>* 9-11位保留给OS使用；
>* 7-8位恒为0；
>* 第6位表示该页是否为dirty，即是否需要在swap out的时候写回外存；
>* 第5位表示是否被访问；
>* 3-4位恒为0；
>* 0-2位分别表示存在位、是否允许读写、访问该页需要的特权级；

可以发现其中保留的3bit，在页交换中可以作为计数器来使用，从而能够实现一些机制和策略。



2.

将发生错误的线性地址保存在寄存器中;

在中断栈中压入EFLAGS，CS, EIP，以及页访问异常码error code，由于ISR一定是运行在内核态下的，因此不需要压入ss和esp以及进行栈的切换；

根据中断描述符表查询到对应页访问异常的ISR，跳转到对应的ISR处执行，接下来将由软件进行处理；



3.

这个算法只需换出没有被访问且没有被修改的页，可以修改swap_out_victim的机制，从当前指针开始扫描链表，如果一个页面是（!access&&!dirty）的，则将其从链表上取下。

被换出的页没有被访问，没有被修改。

假如某物理页对应的所有虚拟页中存在一个或以上dirty的页，则认为这个物理页为dirty，虚拟页中存在一个或以上被访问的页，那么物理页被访问过。

在产生page fault的时候换页。