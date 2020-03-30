#include "bootpack.h"

extern struct FIFO8 keyfifo;
void enable_mouse(void);
void init_keyboard(void);

void HariMain(void) {
  // from asm
  struct BOOTINFO *binfo;
  char s[40], mcursor[256], keybuf[32];
  
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

  fifo8_init(&keyfifo, 32, keybuf);
  io_out8(PIC0_IMR, 0xf9); // キーボードを許可
  io_out8(PIC1_IMR, 0xef); // マウスを許可
  
  init_keyboard();
  enable_mouse();

  int i;
  int t = 16;
  while (1) {
    io_cli(); // 割り込み禁止
    // キューの中身がないとき
    if (fifo8_status(&keyfifo) == 0) {
      io_stihlt();
    } else {
      // キューから1文字取り出す。
      i = fifo8_get(&keyfifo);
      io_sti();
      sprintf(s, "%x", i);
      boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, t, 15, 31);
      putfonts8_asc(binfo->vram, binfo->scrnx, 0, t, COL8_FFFFFF, s);
      // 切り替えがあまりにも速いので1文字で出力すると2byteのキーコードが送られてくることがほとんどわからないので縦に4byte表示させてみた。
      if (t != 64)
        t += 16;
    }
  }
}

// keyboard制御回路(キーボードコントローラ:KBC)が制御命令を受け付けられるようになるまで待つ。
void wait_KBC_sendready(void) {
  for(;;) {
    // 命令を受け付けられるようになると, 0x0064の下から2bitが0になる。
    if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
      break;
    }
  }
}

void enable_mouse(void) {
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	return;
}

void init_keyboard(void) {
  wait_KBC_sendready();
  // モード設定
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
  // Mouseの使用設定が0x47
	io_out8(PORT_KEYDAT, KBC_MODE);
	return;
}
