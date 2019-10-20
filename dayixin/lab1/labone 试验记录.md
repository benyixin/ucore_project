# labone 试验记录

实验环境配置过程当中的问题：

1. 想用git clone从网站上面下载实验所需要的文件资料，用VM虚拟环境设置网络连接的时候发现不论是NET还是桥接都没法连上网络，上网搜寻后可能是VM和VirtuaBOX同时安装的问题。参考了[VM和virtualBox共存问题]<https://blog.csdn.net/hhs49707555/article/details/78153919>和[同时装的影响]<https://blog.csdn.net/xiayu0912/article/details/6085255>，但由于尝试过程中系统蓝屏，重来的时候干脆重装了VM，然后就可以直接上网了。

2. 为了不耽误时间，在解决联网问题的时候主要在virtualBox上面进行实验。进行环境和对应的工具使用时候遇到了一些问题。

   * 按照说明上面用terminal进入到对应的目录下面之后，用make指令生成对应的.out文件，看编译过程，多次尝试却没有用make clean导致再输入命令的时候出现问题。

   * 在鼠标点入qemu界面的时候,整个虚拟机就会卡住，只能重启，目前为找到原因

   * 没有事先设置Makefile和tools里的gdbinit文件，导致后期在调试的时候断点设置错误，或者程序干脆没有断点，和部分输出的错误。

     一下图片都是出现失误的图片

     ![1](C:\Users\lenovo\Desktop\大三上文件\操作系统\lab1\1.PNG)



![2](C:\Users\lenovo\Desktop\大三上文件\操作系统\lab1\2.PNG)





![3](C:\Users\lenovo\Desktop\大三上文件\操作系统\lab1\3.PNG)

## 1. 实验过程当中的问题

目的就是加载bootloader，来完成OS的加载工作。

### 	<font size ="5">1.练习一 </font>

####  			 镜像文件的生成，可以用make V=指令来显示出make过程当中的具体操作。

运用cd命令转到对应的目录下就可以再使用以下指令来得到申城镜像文件的过程。

***因为之前make过一次，再用make V=的时候就用不了了，具体看了makefile文件，知道是编写了对应的代码文件指示了编程的过程，已经执行过了，这个V就是起到把过程打印出来的效果。***

```linux
make V=
```

在操作台上就会显示出make过程当中的指令操作。

```C
+ cc kern/init/init.c
gcc -Ikern/init/ -march=i686 -fno-builtin -fno-PIC -Wall -ggdb -m32 -gstabs -nostdinc  -fno-stack-protector -Ilibs/ -Ikern/debug/ -Ikern/driver/ -Ikern/trap/ -Ikern/mm/ -c kern/init/init.c -o obj/kern/init/init.o
kern/init/init.c:95:1: warning: ‘lab1_switch_test’ defined but not used [-Wunused-function]
 lab1_switch_test(void) {
 ^
+ cc kern/libs/readline.c
gcc -Ikern/libs/ -march=i686 -fno-builtin -fno-PIC -Wall -ggdb -m32 -gstabs -nostdinc  -fno-stack-protector -Ilibs/ -Ikern/debug/ -Ikern/driver/ -Ikern/trap/ -Ikern/mm/ -c kern/libs/readline.c -o obj/kern/libs/readline.o
+ cc kern/libs/stdio.c
gcc -Ikern/libs/ -march=i686 -fno-builtin -fno-PIC -Wall -ggdb -m32 -gstabs -nostdinc  -fno-stack-protector -Ilibs/ -Ikern/debug/ -Ikern/driver/ -Ikern/trap/ -Ikern/mm/ -c kern/libs/stdio.c -o obj/kern/libs/stdio.o
+ cc kern/debug/kdebug.c
gcc -Ikern/debug/ -march=i686 -fno-builtin -fno-PIC -Wall -ggdb -m32 -gstabs -nostdinc  -fno-stack-protector -Ilibs/ -Ikern/debug/ -Ikern/driver/ -Ikern/trap/ -Ikern/mm/ -c kern/debug/kdebug.c -o obj/kern/debug/kdebug.o
kern/debug/kdebug.c:251:1: warning: ‘read_eip’ defined but not used [-Wunused-function]
 read_eip(void) {
 ^
+ cc kern/debug/kmonitor.c
gcc -Ikern/debug/ -march=i686 -fno-builtin -fno-PIC -Wall -ggdb -m32 -gstabs -nostdinc  -fno-stack-protector -Ilibs/ -Ikern/debug/ -Ikern/driver/ -Ikern/trap/ -Ikern/mm/ -c kern/debug/kmonitor.c -o obj/kern/debug/kmonitor.o
+ cc kern/debug/panic.c
gcc -Ikern/debug/ -march=i686 -fno-builtin -fno-PIC -Wall -ggdb -m32 -gstabs -nostdinc  -fno-stack-protector -Ilibs/ -Ikern/debug/ -Ikern/driver/ -Ikern/trap/ -Ikern/mm/ -c kern/debug/panic.c -o obj/kern/debug/panic.o
kern/debug/panic.c: In function ‘__panic’:
kern/debug/panic.c:27:5: warning: implicit declaration of function ‘print_stackframe’ [-Wimplicit-function-declaration]
     print_stackframe();
     ^
+ cc kern/driver/clock.c
gcc -Ikern/driver/ -march=i686 -fno-builtin -fno-PIC -Wall -ggdb -m32 -gstabs -nostdinc  -fno-stack-protector -Ilibs/ -Ikern/debug/ -Ikern/driver/ -Ikern/trap/ -Ikern/mm/ -c kern/driver/clock.c -o obj/kern/driver/clock.o
+ cc kern/driver/console.c
gcc -Ikern/driver/ -march=i686 -fno-builtin -fno-PIC -Wall -ggdb -m32 -gstabs -nostdinc  -fno-stack-protector -Ilibs/ -Ikern/debug/ -Ikern/driver/ -Ikern/trap/ -Ikern/mm/ -c kern/driver/console.c -o obj/kern/driver/console.o
+ cc kern/driver/intr.c
gcc -Ikern/driver/ -march=i686 -fno-builtin -fno-PIC -Wall -ggdb -m32 -gstabs -nostdinc  -fno-stack-protector -Ilibs/ -Ikern/debug/ -Ikern/driver/ -Ikern/trap/ -Ikern/mm/ -c kern/driver/intr.c -o obj/kern/driver/intr.o
+ cc kern/driver/picirq.c
gcc -Ikern/driver/ -march=i686 -fno-builtin -fno-PIC -Wall -ggdb -m32 -gstabs -nostdinc  -fno-stack-protector -Ilibs/ -Ikern/debug/ -Ikern/driver/ -Ikern/trap/ -Ikern/mm/ -c kern/driver/picirq.c -o obj/kern/driver/picirq.o
+ cc kern/trap/trap.c
gcc -Ikern/trap/ -march=i686 -fno-builtin -fno-PIC -Wall -ggdb -m32 -gstabs -nostdinc  -fno-stack-protector -Ilibs/ -Ikern/debug/ -Ikern/driver/ -Ikern/trap/ -Ikern/mm/ -c kern/trap/trap.c -o obj/kern/trap/trap.o
kern/trap/trap.c:14:13: warning: ‘print_ticks’ defined but not used [-Wunused-function]
 static void print_ticks() {
             ^
kern/trap/trap.c:30:26: warning: ‘idt_pd’ defined but not used [-Wunused-variable]
 static struct pseudodesc idt_pd = {
                          ^
+ cc kern/trap/trapentry.S
gcc -Ikern/trap/ -march=i686 -fno-builtin -fno-PIC -Wall -ggdb -m32 -gstabs -nostdinc  -fno-stack-protector -Ilibs/ -Ikern/debug/ -Ikern/driver/ -Ikern/trap/ -Ikern/mm/ -c kern/trap/trapentry.S -o obj/kern/trap/trapentry.o
+ cc kern/trap/vectors.S
gcc -Ikern/trap/ -march=i686 -fno-builtin -fno-PIC -Wall -ggdb -m32 -gstabs -nostdinc  -fno-stack-protector -Ilibs/ -Ikern/debug/ -Ikern/driver/ -Ikern/trap/ -Ikern/mm/ -c kern/trap/vectors.S -o obj/kern/trap/vectors.o
+ cc kern/mm/pmm.c
gcc -Ikern/mm/ -march=i686 -fno-builtin -fno-PIC -Wall -ggdb -m32 -gstabs -nostdinc  -fno-stack-protector -Ilibs/ -Ikern/debug/ -Ikern/driver/ -Ikern/trap/ -Ikern/mm/ -c kern/mm/pmm.c -o obj/kern/mm/pmm.o
+ cc libs/printfmt.c
gcc -Ilibs/ -march=i686 -fno-builtin -fno-PIC -Wall -ggdb -m32 -gstabs -nostdinc  -fno-stack-protector -Ilibs/  -c libs/printfmt.c -o obj/libs/printfmt.o
+ cc libs/string.c
gcc -Ilibs/ -march=i686 -fno-builtin -fno-PIC -Wall -ggdb -m32 -gstabs -nostdinc  -fno-stack-protector -Ilibs/  -c libs/string.c -o obj/libs/string.o
+ ld bin/kernel
ld -m    elf_i386 -nostdlib -T tools/kernel.ld -o bin/kernel  obj/kern/init/init.o obj/kern/libs/readline.o obj/kern/libs/stdio.o obj/kern/debug/kdebug.o obj/kern/debug/kmonitor.o obj/kern/debug/panic.o obj/kern/driver/clock.o obj/kern/driver/console.o obj/kern/driver/intr.o obj/kern/driver/picirq.o obj/kern/trap/trap.o obj/kern/trap/trapentry.o obj/kern/trap/vectors.o obj/kern/mm/pmm.o  obj/libs/printfmt.o obj/libs/string.o
+ cc boot/bootasm.S
gcc -Iboot/ -march=i686 -fno-builtin -fno-PIC -Wall -ggdb -m32 -gstabs -nostdinc  -fno-stack-protector -Ilibs/ -Os -nostdinc -c boot/bootasm.S -o obj/boot/bootasm.o
+ cc boot/bootmain.c
gcc -Iboot/ -march=i686 -fno-builtin -fno-PIC -Wall -ggdb -m32 -gstabs -nostdinc  -fno-stack-protector -Ilibs/ -Os -nostdinc -c boot/bootmain.c -o obj/boot/bootmain.o
+ cc tools/sign.c
gcc -Itools/ -g -Wall -O2 -c tools/sign.c -o obj/sign/tools/sign.o
gcc -g -Wall -O2 obj/sign/tools/sign.o -o bin/sign
+ ld bin/bootblock
ld -m    elf_i386 -nostdlib -N -e start -Ttext 0x7C00 obj/boot/bootasm.o obj/boot/bootmain.o -o obj/bootblock.o
'obj/bootblock.out' size: 472 bytes
build 512 bytes boot sector: 'bin/bootblock' success!
dd if=/dev/zero of=bin/ucore.img count=10000
10000+0 records in
10000+0 records out
5120000 bytes (5.1 MB) copied, 0.057786 s, 88.6 MB/s
dd if=bin/bootblock of=bin/ucore.img conv=notrunc
1+0 records in
1+0 records out
512 bytes (512 B) copied, 0.00481768 s, 106 kB/s
dd if=bin/kernel of=bin/ucore.img seek=1 conv=notrunc
146+1 records in
146+1 records out
74871 bytes (75 kB) copied, 0.000588862 s, 127 MB/s

```

命令之后所显示出的代码，功能分析，也是生成镜像文件的过程。

1. cc boot/bootmain.c
   gcc -Iboot/ -march=i686 -fno-builtin -fno-PIC -Wall -ggdb -m32 -gstabs -nostdinc  -fno-stack-protector -Ilibs/ -Os -nostdinc -c boot/bootmain.c -o obj/boot/bootmain.o

   这一段代码的功能是编译对应的文件，生成对应的.o文件

2. ld -m    elf_i386 -nostdlib -N -e start -Ttext 0x7C00 obj/boot/bootasm.o obj/boot/bootmain.o -o obj/bootblock.o
   'obj/bootblock.out' size: 472 bytes
   build 512 bytes boot sector: 'bin/bootblock' success!

3. dd if=bin/bootblock of=bin/ucore.img conv=notrunc
   1+0 records in
   1+0 records out
   512 bytes (512 B) copied, 0.00481768 s, 106 kB/s

   这一段是DD指令生成对应的镜像文件ucore.img，可见他是先写了一个空的再将东西复制进去。

   这个是镜像文件生成的过程。

   符合规范的硬盘主引导扇区的特征是什么：

   

   从sign.c的代码来看，一个磁盘主引导扇区只有512字节

   ```c
   #include <stdio.h>
   #include <errno.h>
   #include <string.h>
   #include <sys/stat.h>
   
   int
   main(int argc, char *argv[]) {
       struct stat st;
       if (argc != 3) {
           fprintf(stderr, "Usage: <input filename> <output filename>\n");
           return -1;
       }
       if (stat(argv[1], &st) != 0) {
           fprintf(stderr, "Error opening file '%s': %s\n", argv[1], strerror(errno));
           return -1;
       }
       printf("'%s' size: %lld bytes\n", argv[1], (long long)st.st_size);
       if (st.st_size > 510) {
           fprintf(stderr, "%lld >> 510!!\n", (long long)st.st_size);
           return -1;
       }
       char buf[512];
       memset(buf, 0, sizeof(buf));
       FILE *ifp = fopen(argv[1], "rb");
       int size = fread(buf, 1, st.st_size, ifp);
       if (size != st.st_size) {
           fprintf(stderr, "read '%s' error, size is %d.\n", argv[1], size);
           return -1;
       }
       fclose(ifp);
       buf[510] = 0x55;
       buf[511] = 0xAA;
       FILE *ofp = fopen(argv[2], "wb+");
       size = fwrite(buf, 1, 512, ofp);
       if (size != 512) {
           fprintf(stderr, "write '%s' error, size is %d.\n", argv[2], size);
           return -1;
       }
       fclose(ofp);
       printf("build 512 bytes boot sector: '%s' success!\n", argv[2]);
       return 0;
   }
   ```

   
   
一个系统被认为是符合规范的硬盘主引导扇区的特征是：
   
第510个（倒数第二个）字节是0x55，
   第511个（倒数第一个）字节是0xAA。
   
   主要的问题集中在Makefile文件的内容，由于教程当中说到用make-mon但是说为无法识别的语句 
   
   ![1571558754815](C:\Users\lenovo\AppData\Roaming\Typora\typora-user-images\1571558754815.png)
   
   ***

### <font size ="5">2.练习二</font>

1. 单步跟踪BIOS进程时候的卡机问题，执行的时候总是显示没有程序在执行

要求修改gdbinit文件，实际上就是执行gdb的时候默认执行的文件，修改的内容主要是：

file bin/kernel

target remote :1234  //链接qemu来调试

set architecture i8086    //CPU运行的环境是这个

break kern_initc	//断点设置的地方

c

x /2i $pc  //打印指令用的



单步调试设置断点成功



***修改init文件多次，发现主要的问题是对于debug的不熟悉，重点学习的地方是debug的方法，以及debug的思考顺序，调试得到的文件对比.s是相同的，表示它是按照对应的编译文件执行的***



尝试过程与实验零的调试过程相似

发现调式过程当中输出的东西与boot文件下面的bootasm.s文件相同

```asm

    cli                                             
    cld                                            
    xorw %ax, %ax                                   
    movw %ax, %ds                                  
    movw %ax, %es                                   
    movw %ax, %ss                                  
seta20.1:
    inb $0x64, %al                                  code16                                             # Assemble for 16-bit mode
    cli                                             # Disable interrupts
    cld                                             # String operations increment

    # Set up the important data segment registers (DS, ES, SS).
    xorw %ax, %ax                                   # Segment number zero
    movw %ax, %ds                                   # -> Data Segment
    movw %ax, %es                                   # -> Extra Segment
    movw %ax, %ss                                   # -> Stack Segment

    # Enable A20:
    #  For backwards compatibility with the earliest PCs, physical
    #  address line 20 is tied low, so that addresses higher than
    #  1MB wrap around to zero by default. This code undoes this.
seta20.1:
    inb $0x64, %al                                  # Wait for not busy(8042 input buffer empty).
    testb $0x2, %al
    jnz seta20.1

    movb $0xd1, %al                                 # 0xd1 -> port 0x64
    outb %al, $0x64                                 # 0xd1 means: write data to 8042's P2 port

seta20.2:
    inb $0x64, %al                                  # Wait for not busy(8042 input buffer empty).
    testb $0x2, %al
    jnz seta20.2

    movb $0xdf, %al                                 # 0xdf -> port 0x60
    outb %al, $0x60                                 # 0xdf = 11011111, means set P2's A20 bit(the 1 bit) to 1

    # Switch from real to protected mode, using a bootstrap GDT
    # and segment translation that makes virtual addresses
    # identical to physical addresses, so that the
    # effective memory map does not change during the switch.
    lgdt gdtdesc
    movl %cr0, %eax
    orl $CR0_PE_ON, %eax
    movl %eax, %cr0

    # Jump to next instruction, but in 32-bit code segment.
    # Switches processor into 32-bit mode.
    ljmp $PROT_MODE_CSEG, $protcseg

    testb $0x2, %al
    jnz seta20.1

    movb $0xd1, %al                                 
    outb %al, $0x64                                 
seta20.2:
    inb $0x64, %al                                  
    testb $0x2, %al
    jnz seta20.2
    movb $0xdf, %al                                 
    outb %al, $0x60                                 
    lgdt gdtdesc
    movl %cr0, %eax
    orl $CR0_PE_ON, %eax
    movl %eax, %cr0
    ljmp $PROT_MODE_CSEG, $protcseg

```

具体分析代码的作用在练习三



###  3.练习三

从 %cs = 0，$pc = 0x7c00进入，这个就是bootloader的进入的地址

看到bootloader对应的.s文件之下，有

```assembly
seta20.1:
    inb $0x64, %al                                  # Wait for not busy(8042 input buffer empty).
    testb $0x2, %al
    jnz seta20.1

    movb $0xd1, %al                                 # 0xd1 -> port 0x64
    outb %al, $0x64                                 # 0xd1 means: write data to 8042's P2 port

seta20.2:
    inb $0x64, %al                                  # Wait for not busy(8042 input buffer empty).
    testb $0x2, %al
    jnz seta20.2

    movb $0xdf, %al                                 # 0xdf -> port 0x60
    outb %al, $0x60                                 # 0xdf = 11011111, means set P2's A20 bit(the 1 bit) to 1

    # Switch from real to protected mode, using a bootstrap GDT
    # and segment translation that makes virtual addresses
    # identical to physical addresses, so that the
    # effective memory map does not change during the switch.
    lgdt gdtdesc//	GDT表格
    movl %cr0, %eax
    orl $CR0_PE_ON, %eax
    movl %eax, %cr0

    # Jump to next instruction, but in 32-bit code segment.
    # Switches processor into 32-bit mode.
    ljmp $PROT_MODE_CSEG, $protcseg//长跳转到CS的新基址

.code32                                             # Assemble for 32-bit mode
```

从实模式到保护模式

1. 初始化ds，es和ss等段寄存器为0
2. 使能A20门，其中seta20.1写数据到0x64端口，表示要写数据给8042芯片的Output Port;seta20.2写数据到0x60端口，把Output Port的第2位置为1，从而使能A20门。
3. 建立gdt，设置了两个段描述符，分别对应代码段和数据段

```
gdt:
    SEG_NULLASM                                     # null seg
    SEG_ASM(STA_X|STA_R, 0x0, 0xffffffff)           # code seg for bootloader and kernel
    SEG_ASM(STA_W, 0x0, 0xffffffff)                 # data seg for bootloader and kernel
```

1. 使用`lgdt gdtdesc`将gdt的地址加载到GDTR寄存器中
2. 设置cr0寄存器的最低位为1，从而使能保护模式

```
    movl %cr0, %eax
    orl $CR0_PE_ON, %eax
    movl %eax, %cr0
```

1. 执行`ljmp $PROT_MODE_CSEG, $protcseg`从而进入保护模式，其中PROT_MODE_CSEG为8，即代码段的段选择子，执行ljmp时会将cs寄存器设置为PROT_MODE_CSEG=8.

   ***老师所问的问题也集中在这个地方，由于从实模式到保护模式的转换所有的指令的地址也变化的和以前不一样了，所以必须要跳转到新的地址去（实际上就是对原本的指针做了赋值，但是实际存储的地址不变）***

2. protcseg的开头先将除CS外的段寄存器设置为数据段的段选择子，然后调用bootmain来执行bootmain。

```
    # Set up the protected-mode data segment registers
    movw $PROT_MODE_DSEG, %ax                       # Our data segment selector
    movw %ax, %ds                                   # -> DS: Data Segment
    movw %ax, %es                                   # -> ES: Extra Segment
    movw %ax, %fs                                   # -> FS
    movw %ax, %gs                                   # -> GS
    movw %ax, %ss                                   # -> SS: Stack Segment
```



### 4. 练习四

文档见LBA模式

读一个扇区的流程大致如下：

1. 等待磁盘准备好
2. 发出读取扇区的命令
3. 等待磁盘准备好
4. 把磁盘扇区数据读到指定内存

ELE文件（.o文件的常见格式）

可执行文件

读硬盘扇区的代码如下:

```
// bootmain.c
/* readsect - read a single sector at @secno into @dst */
static void
readsect(void *dst, uint32_t secno) {
    // wait for disk to be ready
    waitdisk();

    outb(0x1F2, 1);                         // count = 1
    outb(0x1F3, secno & 0xFF);
    outb(0x1F4, (secno >> 8) & 0xFF);
    outb(0x1F5, (secno >> 16) & 0xFF);
    outb(0x1F6, ((secno >> 24) & 0xF) | 0xE0);
    outb(0x1F7, 0x20);                      // cmd 0x20 - read sectors

    // wait for disk to be ready
    waitdisk();

    // read a sector
    insl(0x1F0, dst, SECTSIZE / 4);
}
```

从`outb()`可以看出这里是用LBA模式的PIO（Program IO）方式来访问硬盘的。从`磁盘IO地址和对应功能表`可以看出，该函数一次只读取一个扇区。

| IO地址 | 功能                                                         |
| ------ | ------------------------------------------------------------ |
| 0x1f0  | 读数据，当0x1f7不为忙状态时，可以读。                        |
| 0x1f2  | 要读写的扇区数，每次读写前，你需要表明你要读写几个扇区。最小是1个扇区 |
| 0x1f3  | 如果是LBA模式，就是LBA参数的0-7位                            |
| 0x1f4  | 如果是LBA模式，就是LBA参数的8-15位                           |
| 0x1f5  | 如果是LBA模式，就是LBA参数的16-23位                          |
| 0x1f6  | 第0~3位：如果是LBA模式就是24-27位 第4位：为0主盘；为1从盘    |
| 0x1f7  | 状态和命令寄存器。操作时先给命令，再读取，如果不是忙状态就从0x1f0端口读数据 |

其中`insl`的实现如下：

```
// x86.h
static inline void
insl(uint32_t port, void *addr, int cnt) {
    asm volatile (
            "cld;"
            "repne; insl;"
            : "=D" (addr), "=c" (cnt)
            : "d" (port), "0" (addr), "1" (cnt)
            : "memory", "cc");
}
```

> bootloader是如何加载ELF格式的OS？

1. 从硬盘读了8个扇区数据到内存`0x10000`处，并把这里强制转换成`elfhdr`使用；
2. 校验`e_magic`字段；
3. 根据偏移量分别把程序段的数据读取到内存中。

### 5. 练习五

首先，可以通过`read_ebp()`和`read_eip()`函数来获取当前ebp寄存器和eip 寄存器的信息。

在push ebp之前会先把调用参数入栈，而ebp长16位(也就是2Byte)，所以`(ebp+2)[0...3]`就是传入参数。

由于函数调用的过程会把上一层的ebp压入栈中，所以当前ebp中存的值正是上一层的ebp。而上一层的eip 事实上已经不是eip了，而是调入这个函数的地方，也就是当前函数的返回地址。

这个有关于函数调用和系统调用的问题，栈指针的变化和出入站的顺序。

实现过程代码如下：

```
void
print_stackframe(void) {
    uint32_t ebp = read_ebp(), eip = read_eip();
    for (int i = 0; i < STACKFRAME_DEPTH && ebp != 0; i++) {
        cprintf("ebp: 0x%08x eip: 0x%08x args:", ebp, eip);
        for (int ij= 0; j < 4; j++) {
            cprintf(" 0x%08x", ((uint32_t*)(ebp + 2))[j]);
        }
        cprintf("\n");
        print_debuginfo(eip - 1);
        eip = *((uint32_t*) ebp + 1);
        ebp = *((uint32_t*) ebp);
    }
}
```

执行 `make qemu`得到如下结果:

```
(THU.CST) os is loading ...

Special kernel symbols:
  entry  0x00100000 (phys)
  etext  0x0010325f (phys)
  edata  0x0010ea16 (phys)
  end    0x0010fd20 (phys)
Kernel executable memory footprint: 64KB
ebp: 0x00007b38 eip: 0x00100a27 args: 0x0d210000 0x00940010 0x00940001 0x7b680001
    kern/debug/kdebug.c:305: print_stackframe+21
ebp: 0x00007b48 eip: 0x00100d21 args: 0x007f0000 0x00000010 0x00000000 0x00000000
    kern/debug/kmonitor.c:125: mon_backtrace+10
ebp: 0x00007b68 eip: 0x0010007f args: 0x00a10000 0x00000010 0x7b900000 0x00000000
    kern/init/init.c:48: grade_backtrace2+19
ebp: 0x00007b88 eip: 0x001000a1 args: 0x00be0000 0x00000010 0x00000000 0x7bb4ffff
    kern/init/init.c:53: grade_backtrace1+27
ebp: 0x00007ba8 eip: 0x001000be args: 0x00df0000 0x00000010 0x00000000 0x00000010
    kern/init/init.c:58: grade_backtrace0+19
ebp: 0x00007bc8 eip: 0x001000df args: 0x00500000 0x00000010 0x00000000 0x00000000
    kern/init/init.c:63: grade_backtrace+26
ebp: 0x00007be8 eip: 0x00100050 args: 0x7d6e0000 0x00000000 0x00000000 0x00000000
    kern/init/init.c:28: kern_init+79
ebp: 0x00007bf8 eip: 0x00007d6e args: 0x7c4f0000 0xfcfa0000 0xd88ec031 0xd08ec08e
    <unknow>: -- 0x00007d6d --
```

###  6.练习六

问题回答：

中断向量表一个表项占用8字节，其中2-3字节是段选择子，0-1字节和6-7字节拼成位移，
两者联合便是中断处理程序的入口地址。

![img](https://images2015.cnblogs.com/blog/615450/201701/615450-20170101141419586-1012406281.png)

这个是表项的结构图：其中第16到31位为中断例程的段选择子，第0到15位 和 第48到63位分别为偏移量的地位和高位。这几个数据一起决定了中断处理代码的入口地址。

以下是具体的代码：

```C
    extern uintptr_t __vectors[];  // (1) 拿到外部变量 __vector
    int i;
// (2) 使用SETGATE宏，对中断描述符表中的每一个表项进行设置
    for (i = 0; i < sizeof(idt) / sizeof(struct gatedesc); i ++) {
        SETGATE(idt[i], 0, GD_KTEXT, __vectors[i], DPL_KERNEL);
    } 
	// set for switch from user to kernel（源文件的，表示转换）
    SETGATE(idt[T_SWITCH_TOU], 0, GD_KTEXT, __vectors[T_SWITCH_TOU], DPL_USER);
    SETGATE(idt[T_SWITCH_TOK], 0, GD_KTEXT, __vectors[T_SWITCH_TOK], DPL_USER);
    // (3) 调用lidt函数，设置中断描述符表
    lidt(&idt_pd);
```



在trap.c文件当中的idt_init函数

```C
//在函数体头部声明一个静态变量用于计数
static int32_t tick_count = 0;
//然后，在时间中断 IRQ_OFFSET + IRQ_TIMER的case中添加判断打印的条件：

tick_count++;
        if (0 == (tick_count % TICK_NUM)) {
            print_ticks();
        }
```

这个函数类似于标签作用，

###  7. 测试

按照实验回答当中的指导将代码敲入到trap_dispatch当中，使得其在每个时钟中断就会响应输出。

具体代码在文件中。