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