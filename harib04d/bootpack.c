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
  int t = 16;
  while (1) {
    io_cli(); // 割り込み禁止
    if (keybuf.len == 0) {
      io_stihlt();
    } else {
      // 1回のループで全ての文字を読まない
      // 1文字が表示されるだけになるはず。
      i = keybuf.data[keybuf.next_r];
      keybuf.len--;
      keybuf.next_r++;
      if (keybuf.next_r == 32) {
        keybuf.next_r = 0;
      }
      sprintf(s, "%x", i);
      io_sti();
      boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, t, 15, 31);
      putfonts8_asc(binfo->vram, binfo->scrnx, 0, t, COL8_FFFFFF, s);
      // 切り替えがあまりにも速いので1文字で出力すると2byteのキーコードが送られてくることがほとんどわからないので縦に4byte表示させてみた。
      if (t != 64)
        t += 16;
    }
  }
}

