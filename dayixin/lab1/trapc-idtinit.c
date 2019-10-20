//在函数体头部声明一个静态变量用于计数
static int32_t tick_count = 0;
//然后，在时间中断 IRQ_OFFSET + IRQ_TIMER的case中添加判断打印的条件：

tick_count++;
        if (0 == (tick_count % TICK_NUM)) {
            print_ticks();
        }