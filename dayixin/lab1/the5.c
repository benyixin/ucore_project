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