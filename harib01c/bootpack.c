extern void io_hlt(void);
extern void write_mem8(int addr, int data);

void HariMain(void) {
  int i;
  char* p;

  // VRAMのメモリマップは0xa0000から。
  for (i = 0xa0000; i <= 0xaffff; ++i) {
    p = (char *) i; // メモリアドレスを入れる。
    *p = i & 0x0f;
  }

  while (1) {
    io_hlt();
  }
}