/**
 * hairb05c Analyze mouse input
 */ 

#include "bootpack.h"

// buf → mouseのバイト数, phase → 0xfaを読む前？何バイト目？
struct MOUSE_DEC {
  unsigned char buf[3], phase;
  int x, y, btn;
};

extern struct FIFO8 keyfifo;
extern struct FIFO8 mousefifo;
void init_keyboard(void);
void enable_mouse(struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat);

void HariMain(void) {
  // from asm
  struct BOOTINFO *binfo;
  char s[40], mcursor[256], keybuf[32], mousebuf[32];
  unsigned char mouse_dbuf[3], mouse_phase;
  struct MOUSE_DEC mdec;
  
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

  fifo8_init(&keyfifo, 32, keybuf);
  fifo8_init(&mousefifo, 32, mousebuf);
  io_out8(PIC0_IMR, 0xf9); // キーボードを許可
  io_out8(PIC1_IMR, 0xef); // マウスを許可
  
  init_keyboard();
  enable_mouse(&mdec);

  int i;
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
        sprintf(s+5, "%d    %d", mdec.x, mdec.y);
        boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 32 + 8 * 8 - 1, 31);
        putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
      }
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

void enable_mouse(struct MOUSE_DEC *mdec) {
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
  mdec->phase = 0;
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

int mouse_decode(struct MOUSE_DEC *mdec, unsigned char dat) {
  // harib05aをモジュール化
  if (mdec->phase == 0) {
    if (dat == 0xfa) {
      mdec->phase = 1;
    }
    // keyboard 制御回路が初期化できていない
    return 0;
  } else if (mdec->phase == 1) {
    if ((dat & 0xc8) == 0x08) {
      mdec->buf[0] = dat;
      mdec->phase = 2;
    }
    return 0;
  } else if (mdec->phase == 2) {
    mdec->buf[1] = dat;
    mdec->phase = 3;
    return 0;
  } else if (mdec->phase == 3) {
    mdec->buf[2] = dat;
    mdec->phase = 1;
    mdec->btn = mdec->buf[0] & 0x07;
    mdec->x = mdec->buf[1];
    mdec->y = mdec->buf[2];
    if ((mdec->buf[0] & 0x10) != 0) {
      mdec->x |= 0xffffff00;
    }
    if ((mdec->buf[0] & 0x20) != 0) {
      mdec->y |= 0xffffff00;
    }
    mdec->y = - mdec->y; // マウスではy方向の符号が画面と反対
    return 1;
  }

  return -1; // ここに来ることはない(はず)
}