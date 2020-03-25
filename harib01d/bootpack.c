// pointerのエイリアステスト

extern void io_hlt(void);
extern void write_mem8(int addr, int data);

void HariMain(void) {
  int i;
  char* p = (char*) 0xa0000;

  // VRAMのメモリマップは0xa0000から。
  for (i = 0; i <= 0xffff; ++i) {
    *(p + i) = i & 0x0f;
  }

  while (1) {
    io_hlt();
  }
}