/**
 * hairb06c memset_subをアセンブラで記述する。
 */ 

#include "bootpack.h"

extern struct FIFO8 keyfifo;
extern struct FIFO8 mousefifo;

#define EFLAGS_AC_BIT       0x0004000
#define CR0_CACHE_DISABLE   0x6000000

#define MEMMAN_FREES 4090
#define MEMMAN_ADDR  0x003c0000

struct FREEINFO { // 空き情報
  unsigned int addr, size;
};

struct MEMMAN { // メモリ管理
  int frees, maxfrees, lostsize, losts;
  struct FREEINFO free[MEMMAN_FREES];
};

unsigned int memtest(unsigned int start, unsigned int end);
extern unsigned int memtest_sub(unsigned int start, unsigned int end);

void memman_init(struct MEMMAN *man);
unsigned int memman_total(struct MEMMAN *man);
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size);

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

// メモリマネージャの要素を初期化
void memman_init(struct MEMMAN *man) {
  man->frees = 0; // 空き情報の個数
  man->maxfrees = 0; // 状況観察用
  man->lostsize = 0; // 解放に失敗した合計サイズ
  man->losts = 0; // 解放に失敗した回数
}

// 空きサイズの合計を報告
unsigned int memman_total(struct MEMMAN *man) {
	unsigned int i, t = 0;
	for (i = 0; i < man->frees; i++) {
		t += man->free[i].size;
	}
	return t;
}

// メモリの確保を行う。
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size) {
  unsigned int i, a;
  for (i = 0; i < man->frees; i++) {
		if (man->free[i].size >= size) {
			/* 十分な広さのあきを発見 */
			a = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;
			if (man->free[i].size == 0) {
				/* free[i]がなくなったので前へつめる */
				man->frees--;
				for (; i < man->frees; i++) {
					man->free[i] = man->free[i + 1]; /* 構造体の代入 */
				}
			}
			return a;
		}
	}
	return 0; /* あきがない */
}

// メモリを解放する。
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size) {
	int i, j;
	/* まとめやすさを考えると、free[]がaddr順に並んでいるほうがいい */
	/* だからまず、どこに入れるべきかを決める */
	for (i = 0; i < man->frees; i++) {
		if (man->free[i].addr > addr) {
			break;
		}
	}
	/* free[i - 1].addr < addr < free[i].addr */
	if (i > 0) {
		/* 前がある */
		if (man->free[i - 1].addr + man->free[i - 1].size == addr) {
			/* 前のあき領域にまとめられる */
			man->free[i - 1].size += size;
			if (i < man->frees) {
				/* 後ろもある */
				if (addr + size == man->free[i].addr) {
					/* なんと後ろともまとめられる */
					man->free[i - 1].size += man->free[i].size;
					/* man->free[i]の削除 */
					/* free[i]がなくなったので前へつめる */
					man->frees--;
					for (; i < man->frees; i++) {
						man->free[i] = man->free[i + 1]; /* 構造体の代入 */
					}
				}
			}
			return 0; /* 成功終了 */
		}
	}
	/* 前とはまとめられなかった */
	if (i < man->frees) {
		/* 後ろがある */
		if (addr + size == man->free[i].addr) {
			/* 後ろとはまとめられる */
			man->free[i].addr = addr;
			man->free[i].size += size;
			return 0; /* 成功終了 */
		}
	}
	/* 前にも後ろにもまとめられない */
	if (man->frees < MEMMAN_FREES) {
		/* free[i]より後ろを、後ろへずらして、すきまを作る */
		for (j = man->frees; j > i; j--) {
			man->free[j] = man->free[j - 1];
		}
		man->frees++;
		if (man->maxfrees < man->frees) {
			man->maxfrees = man->frees; /* 最大値を更新 */
		}
		man->free[i].addr = addr;
		man->free[i].size = size;
		return 0; /* 成功終了 */
	}
	/* 後ろにずらせなかった */
	man->losts++;
	man->lostsize += size;
	return -1; /* 失敗終了 */
}
