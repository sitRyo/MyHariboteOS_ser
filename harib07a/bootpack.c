/**
 * hairb06c memset_subをアセンブラで記述する。
 */ 

#include "bootpack.h"

extern struct FIFO8 keyfifo;
extern struct FIFO8 mousefifo;

void HariMain(void) {
  // from asm
  struct BOOTINFO *binfo;
  char s[40], mcursor[256], keybuf[32], mousebuf[32];
  unsigned char mouse_dbuf[3], mouse_phase;
  struct MOUSE_DEC mdec;
  int i;
  unsigned int memtotal;
  struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
  
  binfo = (struct BOOTINFO *) 0x0ff0;

  init_gdtidt();
  init_pic();
  io_sti(); // IDT/PICの初期化が終わっているので割り込みフラグを立たせる。

  init_palette();
  init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
  int mx = (binfo->scrnx - 16) / 2;
  int my = (binfo->scrny - 28 - 16) / 2;
  
  init_mouse_cursor8(mcursor, COL8_008484);
  putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
  sprintf(s, "(%d, %d)", mx, my);
  putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);

  memtotal = memtest(0x00400000, 0xbfffffff);
  memman_init(memman);
  memman_free(memman, 0x00001000, 0x0009e000);
  memman_free(memman, 0x00400000, memtotal - 0x00400000);

  fifo8_init(&keyfifo, 32, keybuf);
  fifo8_init(&mousefifo, 32, mousebuf);
  io_out8(PIC0_IMR, 0xf9); // キーボードを許可
  io_out8(PIC1_IMR, 0xef); // マウスを許可
  
  init_keyboard();
  enable_mouse(&mdec);

  sprintf(s, "memory %dMB free: %dKB", memtotal / (1024 * 1024), memman_total(memman) / 1024);
  putfonts8_asc(binfo->vram, binfo->scrnx, 0, 32, COL8_FFFFFF, s);

  // int t = 16;
  mouse_phase = 0;
  while (1) {
    io_cli(); // 割り込み禁止
    // キューの中身がないとき
    if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {
      io_stihlt();
    } else if (fifo8_status(&keyfifo) != 0) {
      // キューから1文字取り出す。
      i = fifo8_get(&keyfifo);
      io_sti();
      sprintf(s, "%x", i);
      boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
      putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
      // 切り替えがあまりにも速いので1文字で出力すると2byteのキーコードが送られてくることがほとんどわからないので縦に4byte表示させてみた。
      //if (t != 64)
      //        t += 16;
    } else if (fifo8_status(&mousefifo) != 0) {
      i = fifo8_get(&mousefifo);
      io_sti();
      if (mouse_decode(&mdec, (unsigned char) i) != 0) {
        // sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
        sprintf(s, "[lcr "); // s[0] = [, s[1] = l, s[2] = c, s[3] = r, s[4] = ' '
        // 0x0001(左のボタン)
        if ((mdec.btn & 0x01) != 0) {
          s[1] = 'L';
        }
        // 0x0010(右のボタン)
        if ((mdec.btn & 0x02) != 0) {
          s[3] = 'R';
        }
        // 0x0100(真ん中のボタン)
        if ((mdec.btn & 0x04) != 0) {
          s[2] = 'C';
        }
        sprintf(s+5, "%d %d", mdec.x, mdec.y);
        
        boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 32 + 8 * 8 - 1, 31);
        putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);

        boxfill8(binfo->vram, binfo->scrnx, COL8_008484, mx, my, mx + 15, my + 15); /* マウス消す */
        mx += mdec.x;
        my += mdec.y;
        if (mx < 0) {
          mx = 0;
        }
        if (my < 0) {
          my = 0;
        }
        if (mx > binfo->scrnx - 16) {
          mx = binfo->scrnx - 16;
        }
        if (my > binfo->scrny - 16) {
          my = binfo->scrny - 16;
        }
        sprintf(s, "(%d, %d)", mx, my);
        // マウスカーソルを表示し直す処理はそれまでマウスがあった座標を塗りつぶして消して、新しい座標にマウスを書き直す。
        boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 0, 79, 15); /* 座標消す */
				putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s); /* 座標書く */
				putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16); /* マウス描く */
      }
    }
  }
}
