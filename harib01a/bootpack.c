extern void io_hlt(void);
extern void write_mem8(int addr, int data);

void HariMain(void) {
  int i;

  // VRAMのメモリマップは0xa0000から。
  for (i = 0xa0000; i <= 0xaffff; ++i) {
    write_mem8(i, 15);
  }

  while (1) {
    io_hlt();
  }
}