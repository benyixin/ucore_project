# Lab2

## lx0

使用diff和patch命令进行了代码的合并

```
diff -b lab1/kern/trap/trap.c lab2/kern/trap/trap.c > tem.patch

patch -p0 < tem.patch
```

这两句命令是比较两份.c文件的不同，构造文件补丁，然后将其以打补丁形式组合在一起（以后者覆盖前者）

其中-b是无视一个以上空格的意思，tem是生成的临时补丁，-p0表示忽略0级目录（即直接使用该当前目录，根据需要可以有p1,p2,pN等多种）。文件名可以替换成lab1所有涉及的文件。

## lx1

经过查询资料及提示，可以认为first-fit是以下的一个过程：

> 要求空闲分区链以地址递增的次序链接。在分配内存时，从链首开始顺序查找，直至找到一个大小能满足要求的分区为止；然后再按照作业的大小，从该分取中划出一块内存空间分配给请求者，余下的空闲分区仍留在空闲链中。若从链首直到链尾都不能找到一个能满足要求的分区，则此次内存分配失败，返回。

所需要修改的函数位于default_pmm.c，先阅读代码memlayout.h进行分析,以下是page的结构

```c
struct Page {
    int ref;                        // page frame's reference counter
    uint32_t flags;                 // array of flags that describe the status of the page frame
    unsigned int property;          // the num of free block, used in first fit pm manager
    list_entry_t page_link;         // free list link
};
```

这个结构体的ref是引用计数器（可以理解为编号），数组flags（事实上是32位的bit）描述对应的内存是否被占用，空闲块数property，以及维护空闲页的链表入口指针（链表相对于这个结构体，可以认为是全局static的，任意多的page共享一个链表）。



default_alloc_pages为从空闲链表中申请n个空闲页，如果成功，返回第一个页表的地址。

```c
default_alloc_pages(size_t n) {
    assert(n > 0);
    if (n > nr_free) {
        return NULL;
    }
    struct Page *page = NULL;
    list_entry_t *le = &free_list;
    while ((le = list_next(le)) != &free_list) {
        struct Page *p = le2page(le, page_link);
        if (p->property >= n) {
            page = p;
            break;
        }
    }
    if (page != NULL) {
        list_del(&(page->page_link));
        if (page->property > n) {
            struct Page *p = page + n;
            p->property = page->property - n;
            list_add(&free_list, &(p->page_link));
    }
        nr_free -= n;
        ClearPageProperty(page);
    }
    return page;
}
```

大意为对一个合法的大于0小于空项总数的页表数量n，选free_list链表的起点，while循环的条件为le所指的地址不为free_list的起点，即没有转满一周，对每个可行的链表节点le，分配所指地址给临时的页p，如果这个页后面有大于等于n个空闲页，那么将这些页赋值给page并结束循环。如果能够找到这样的页，那么从全局链表free_list中删去对应的节点，如果空闲区域大于n，则将差值补回free_list的该位置，重新计算相应的property的值。但是原来的实现方式是有问题的，经过list_del后free_list将会不存在page_link这个地址对应的节点，应当使用先插入后删除的方式，将list_del以到nr_free-=n之前。



此为释放一组页表的函数，将释放的页重新加入到页链表中。

```c
default_free_pages(struct Page *base, size_t n) {
    assert(n > 0);
    struct Page *p = base;
    for (; p != base + n; p ++) {
        assert(!PageReserved(p) && !PageProperty(p));
        p->flags = 0;
        set_page_ref(p, 0);
    }
    base->property = n;
    SetPageProperty(base);
    list_entry_t *le = list_next(&free_list);
    while (le != &free_list) {
        p = le2page(le, page_link);
        le = list_next(le);
        if (base + base->property == p) {
            base->property += p->property;
            ClearPageProperty(p);
            list_del(&(p->page_link));
        }
        else if (p + p->property == base) {
            p->property += base->property;
            ClearPageProperty(base);
            base = p;
            list_del(&(p->page_link));
        }
    }
    nr_free += n;
    list_add(&free_list, &(base->page_link));
}
```

对一个大于0的合法的n，将参数的基地址（base指针所指，从这一页开始释放，逻辑上可以称为基地址）传入并初始化一个page，循环n次，在无异常的情况下将标志位置0（要判断原先这些物理页是否真的被占用了，如果释放未被占用的物理页，这就是属于出现了异常）

然后对base页设置property，置标志位为0和获取一个free_list的起点并开始遍历（还是只遍历一周以内），考虑是否存在可以合并的相邻页块，如果有则将之删去，并将属性加在base页块上。if和else if中的条件为地址相同（页连续），即前后各搜寻一次，如果前后都有则二者在p自增过程中可以先后被合并。最后空闲量+n，将归并好的页块插入链表已经搜索好的位置。

//warning answer的东西line182-190是干嘛的

考虑到时间复杂度，插入节点时循环搜索链表来寻找应当插入的位置，往往会导致需要近乎遍历链表，最坏情况下付出线性复杂度，因此，如果在链表之上构造平衡搜索二叉树，在树的非根节点保存其右子树的最小块的地址信息，则可以实现效率的提升，如图是我对于这一类方案的设计示意图，复杂度降为对数级。

![IMG_2091105_032435](D:\课件2019\操作系统\实验\IMG_2091105_032435.jpg)



# lx2

需要我们补全的地方如下

```c
get_pte(pde_t *pgdir, uintptr_t la, bool create) {
    /* LAB2 EXERCISE 2: YOUR CODE
     *
     * If you need to visit a physical address, please use KADDR()
     * please read pmm.h for useful macros
     *
     * Maybe you want help comment, BELOW comments can help you finish the code
     *
     * Some Useful MACROs and DEFINEs, you can use them in below implementation.
     * MACROs or Functions:
     *   PDX(la) = the index of page directory entry of VIRTUAL ADDRESS la.
     *   KADDR(pa) : takes a physical address and returns the corresponding kernel virtual address.
     *   set_page_ref(page,1) : means the page be referenced by one time
     *   page2pa(page): get the physical address of memory which this (struct Page *) page  manages
     *   struct Page * alloc_page() : allocation a page
     *   memset(void *s, char c, size_t n) : sets the first n bytes of the memory area pointed by s
     *                                       to the specified value c.
     * DEFINEs:
     *   PTE_P           0x001                   // page table/directory entry flags bit : Present
     *   PTE_W           0x002                   // page table/directory entry flags bit : Writeable
     *   PTE_U           0x004                   // page table/directory entry flags bit : User can access
     */
#if 0
    pde_t *pdep = NULL;   // (1) find page directory entry
    if (0) {              // (2) check if entry is not present
                          // (3) check if creating is needed, then alloc page for page table
                          // CAUTION: this page is used for page table, not for common data page
                          // (4) set page reference
        uintptr_t pa = 0; // (5) get linear address of page
                          // (6) clear page content using memset
                          // (7) set page directory entry's permission
    }
    return NULL;          // (8) return page table entry
#endif
}         // (8) return page table entry
```

注释很长但是代码事实上什么都没有做（代码都在if 0里）

```c
pte_t *
get_pte(pde_t *pgdir, uintptr_t la, bool create) {
        pde_t *pdep = &pgdir[PDX(la)];  //找页表地址
        if (!(*pdep & PTE_P)) { //如果获取不成功
            struct Page *page;
            if (!create || (page = alloc_page()) == NULL) { //假如不需要分配或是分配失败
                return NULL;
        }
        set_page_ref(page, 1); //引用次数加一
        uintptr_t pa = page2pa(page);  //得到该页物理地址
        memset(KADDR(pa), 0, PGSIZE); //物理地址转虚拟地址，并初始化
        *pdep = pa | PTE_U | PTE_W | PTE_P; //设置控制位
    }
    return &((pte_t *)KADDR(PDE_ADDR(*pdep)))[PTX(la)]; 
    //KADDR(PDE_ADDR(*pdep)):这部分是由页目录项地址得到关联的页表物理地址， 再转成虚拟地址
    //PTX(la)：返回虚拟地址la的页表项索引
    //最后返回的是虚拟地址la对应的页表项入口地址
}
```



>PDE(目录项，32位)的各部分含义如下：
>
>前20位表示4K对齐的该PDE对应的页表起始位置（物理地址，该物理地址的高20位即PDE中的高20位，低12位为0）；
>
>第9-11位未被CPU使用，可保留给OS使用；
>
>接下来的第8位可忽略；
>
>第7位用于设置Page大小，0表示4KB；
>
>第6位恒为0；
>
>第5位用于表示该页是否被使用过；
>
>第4位设置为1则表示不对该页进行缓存；
>
>第3位设置是否使用write through缓存写策略；
>
>第2位表示该页的访问需要的特权级；
>
>第1位表示是否允许读写；
>
>第0位为该PDE的存在位；
>
>PTE(页表，32位)各部分含义如下:
>
>高20位与PDE相似的，用于表示该PTE指向的物理页的物理地址；
>
>9-11位保留给OS使用；
>
>7-8位恒为0；
>
>第6位表示该页是否为dirty，即是否需要在swap out的时候写回外存；
>
>第5位表示是否被访问；
>
>3-4位恒为0；
>
>0-2位分别表示存在位、是否允许读写、访问该页需要的特权级；



>- 页访问异常时硬件需要完成的事情分别如下：
>    - 将发生错误的线性地址保存在cr2寄存器中;
>    - 在中断栈中依次压入EFLAGS，CS, EIP，以及页访问异常码error code，如果page fault是发生在用户态，则还需要先压入ss和esp，并且切换到内核栈；
>    - 根据中断描述符表查询到对应page fault的ISR，跳转到对应的ISR处执行，接下来将由软件进行page fault处理；

在实现一个把错误信息尽可能输出的机制，并可以从软件得到地址，异常码，异常时的堆栈结构。



# lx3

以下是page_remove_pte函数，和练习2一样是空的

```c
static inline void
page_remove_pte(pde_t *pgdir, uintptr_t la, pte_t *ptep) {
    /* LAB2 EXERCISE 3: YOUR CODE
     *
     * Please check if ptep is valid, and tlb must be manually updated if mapping is updated
     *
     * Maybe you want help comment, BELOW comments can help you finish the code
     *
     * Some Useful MACROs and DEFINEs, you can use them in below implementation.
     * MACROs or Functions:
     *   struct Page *page pte2page(*ptep): get the according page from the value of a ptep
     *   free_page : free a page
     *   page_ref_dec(page) : decrease page->ref. NOTICE: ff page->ref == 0 , then this page should be free.
     *   tlb_invalidate(pde_t *pgdir, uintptr_t la) : Invalidate a TLB entry, but only if the page tables being
     *                        edited are the ones currently in use by the processor.
     * DEFINEs:
     *   PTE_P           0x001                   // page table/directory entry flags bit : Present
     */
#if 0
    if (0) {                      //(1) check if this page table entry is present
        struct Page *page = NULL; //(2) find corresponding page to pte
                                  //(3) decrease page reference
                                  //(4) and free this page when page reference reachs 0
                                  //(5) clear second page table entry
                                  //(6) flush tlb
    }
#endif
}
```

根据提示我们补充以下代码

```c
if (*ptep & PTE_P) {//如果这个二级页表项存在
        struct Page *page = pte2page(*ptep);//获取对应物理页的page
        if (page_ref_dec(page) == 0) {//如果没有虚拟页指向这个物理页
            free_page(page);//释放该页
        }
        *ptep = 0;//二级页表存在位置0，表示该映射关系无效
        tlb_invalidate(pgdir, la);//刷新TLB
    }
```

