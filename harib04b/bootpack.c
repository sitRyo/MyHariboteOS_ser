#include "bootpack.h"

extern struct KEYBUF keybuf;

void HariMain(void) {
  // from asm
  struct BOOTINFO *binfo;
  char s[40], mcursor[256];
  
  binfo = (struct BOOTINFO *) 0x0ff0;

  init_gdtidt();
  init_pic();
  io_sti(); // IDT/PICの初期化が終わっているので割り込みフラグを立たせる。

  init_palette();
  init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
  int mx = (binfo->scrnx - 16) / 2; // ��ʒ����ɂȂ�悤�Ɍv�Z
  int my = (binfo->scrny - 28 - 16) / 2;
  
  init_mouse_cursor8(mcursor, COL8_008484);
  putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
  sprintf(s, "(%d, %d)", mx, my);
  putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);

  io_out8(PIC0_IMR, 0xf9); // キーボードを許可
  io_out8(PIC1_IMR, 0xef); // マウスを許可
  
  int i;
  while (1) {
    io_cli(); // 割り込みフラグを解除
    if (keybuf.flag == 0) {
      // stiしたあとにhltする。CPUの特別命令。stiしてhltする前に割り込みが来ると割り込みを処理する前に
      // CPUをhltしてしまうのでio_stihltを使う。これはstiとhltの間に割り込みが入らない特別な命令。
      io_stihlt(); 
    } else {
      i = keybuf.data;
      keybuf.flag = 0;
      io_sti(); // 割り込みフラグ解除
      sprintf(s, "%x", i); // 参考にしたsprintfを使っているのでxで表示させる。va_listの仕様を理解しなければと思った。
      boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
      putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
    }
  }
}

