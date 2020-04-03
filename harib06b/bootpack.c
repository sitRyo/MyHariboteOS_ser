/**
 * hairb06b
 */ 

#include "bootpack.h"

extern struct FIFO8 keyfifo;
extern struct FIFO8 mousefifo;

#define EFLAGS_AC_BIT       0x0004000
#define CR0_CACHE_DISABLE   0x6000000

unsigned int memtest(unsigned int start, unsigned int end);
unsigned int memtest_sub(unsigned int start, unsigned int end);

void HariMain(void) {
  // from asm
  struct BOOTINFO *binfo;
  char s[40], mcursor[256], keybuf[32], mousebuf[32];
  unsigned char mouse_dbuf[3], mouse_phase;
  struct MOUSE_DEC mdec;
  int i;
  
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

  i = memtest(0x00400000, 0xbfffffff) / (1024 * 1024);
  sprintf(s, "memory %dMB", i);
  putfonts8_asc(binfo->vram, binfo->scrnx, 0, 32, COL8_FFFFFF, s);

  fifo8_init(&keyfifo, 32, keybuf);
  fifo8_init(&mousefifo, 32, mousebuf);
  io_out8(PIC0_IMR, 0xf9); // キーボードを許可
  io_out8(PIC1_IMR, 0xef); // マウスを許可
  
  init_keyboard();
  enable_mouse(&mdec);

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

unsigned int memtest(unsigned int start, unsigned int end) {
  char flg486 = 0;
	unsigned int eflg, cr0, i;

  // AC-bitをチェックして386か, 486以降なのかを調査する。
  // 386(キャッシュメモリが存在しない)ではACBITを1にしても自動で0に戻ってしまう。
	eflg = io_load_eflags();

  // EFLAGSのEFLAGS_AC_BITを1にする演算
	eflg |= EFLAGS_AC_BIT; 

  // EFLAGSをもとに戻す。
	io_store_eflags(eflg);

  // 保存したEFLAGS(AC-bitを1にしたEFLAGS)を読み出す。
	eflg = io_load_eflags();

  // CPUが386だったら0になっている
	if ((eflg & EFLAGS_AC_BIT) != 0) { 
		flg486 = 1;
	}

  // AC-bitを0にもどす
	eflg &= ~EFLAGS_AC_BIT;
	io_store_eflags(eflg);

	if (flg486 != 0) {
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE; 
		store_cr0(cr0);
	}

	i = memtest_sub(start, end);

	if (flg486 != 0) {
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE; 
		store_cr0(cr0);
	}

	return i;
}

unsigned int memtest_sub(unsigned int start, unsigned int end) {
	unsigned int i, *p, old, pat0 = 0xaa55aa55, pat1 = 0x55aa55aa;
	for (i = start; i <= end; i += 0x1000) {
		p = (unsigned int *) (i + 0xffc); // 末尾4byteだけ調べる
		old = *p;
		*p = pat0;			
		*p ^= 0xffffffff;	
		if (*p != pat1) {	
not_memory:
			*p = old;
			break;
		}
		*p ^= 0xffffffff;	
		if (*p != pat0) {	
			goto not_memory;
		}
		*p = old;
	}

  // メモリが存在する場所までのメモリ番地を返す。
	return i;
}