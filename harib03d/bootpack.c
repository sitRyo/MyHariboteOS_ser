#include "bootpack.h"


void HariMain(void) {
  // from asm
  struct BOOTINFO *binfo;
  char s[40], mcursor[256];
  
  binfo = (struct BOOTINFO *) 0x0ff0;

  init_gdtidt();
  init_pic();

  init_palette();
  init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
  int mx = (binfo->scrnx - 16) / 2; // ��ʒ����ɂȂ�悤�Ɍv�Z
  int my = (binfo->scrny - 28 - 16) / 2;
  
  init_mouse_cursor8(mcursor, COL8_008484);
  putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
  sprintf(s, "(%d, %d)", mx, my);
  putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
  
  while (1) {
    io_hlt();
  }
}

