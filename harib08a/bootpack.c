/**
 * hairb08a マウスを画面の外に少し出す。
 */ 

#include "bootpack.h"

extern struct FIFO8 keyfifo;
extern struct FIFO8 mousefifo;

void HariMain(void) {
  struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	char s[40], keybuf[32], mousebuf[128];
	int mx, my, i;
	unsigned int memtotal;
	struct MOUSE_DEC mdec;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse;
	unsigned char *buf_back, buf_mouse[256];

  // gdt(Segment Descriptor), idt(Inttrupt Descriptor)設定。
  // segment 2(だったはず)にidtを保存
	init_gdtidt();

  // picを初期化(HWからの割り込み初期化)
	init_pic();

  /* IDT/PICの初期化が終わったのでCPUの割り込み禁止を解除 */
  // Inttruptフラグを1に。
	io_sti();
	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);
	io_out8(PIC0_IMR, 0xf9); /* PIC1とキーボードを許可(11111001) */
	io_out8(PIC1_IMR, 0xef); /* マウスを許可(11101111) */

	init_keyboard();
	enable_mouse(&mdec);
	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
	memman_free(memman, 0x00400000, memtotal - 0x00400000);

  // 色情報を初期化。
  // vramに色情報を設定。
	init_palette();

  // sheetctlを初期化。最初は何のWindowもない。
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);

  // 下地のWindow
	sht_back  = sheet_alloc(shtctl);

  // Mouse情報
	sht_mouse = sheet_alloc(shtctl);

  // 下地のメモリ確保
	buf_back  = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);

  // 下地、マウスの設定
  // SHEETのbufのポインタをbuf_backのポインタに設定する。
  // SHEET, buffer, サイズx, サイズy, 表示しない色
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); /* 透明色なし */
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);

	init_screen(buf_back, binfo->scrnx, binfo->scrny);
	init_mouse_cursor8(buf_mouse, 99);

  // slideでシートのx,yを設定。その後全部リフレッシュ。
  // slideにSHEET構造体とvx, vyを入れると、SHEETの開始座標がvx, vyに設定されてsheet_backのbufがvx, vyからサイズだけ描画し直される。
	sheet_slide(shtctl, sht_back, 0, 0);
	mx = (binfo->scrnx - 16) / 2; /* 画面中央になるように座標計算 */
	my = (binfo->scrny - 28 - 16) / 2;
	sheet_slide(shtctl, sht_mouse, mx, my);

  // SHEETの描画順番を指定
	sheet_updown(shtctl, sht_back,  0);
	sheet_updown(shtctl, sht_mouse, 1);
	sprintf(s, "(%d, %d)", mx, my);
	putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
	sprintf(s, "memory %dMB   free : %dKB",
			memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts8_asc(buf_back, binfo->scrnx, 0, 32, COL8_FFFFFF, s);
	sheet_refresh(shtctl, sht_back, 0, 0, binfo->scrnx, 48);

  // int t = 16;
  // mouse_phase = 0;
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
      boxfill8(buf_back, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
      putfonts8_asc(buf_back, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
      // 切り替えがあまりにも速いので1文字で出力すると2byteのキーコードが送られてくることがほとんどわからないので縦に4byte表示させてみた。
      //if (t != 64)
      //        t += 16;
			sheet_refresh(shtctl, sht_back, 0, 0, binfo->scrnx, 48);
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
        
        boxfill8(buf_back, binfo->scrnx, COL8_008484, 32, 16, 32 + 8 * 8 - 1, 31);
        putfonts8_asc(buf_back, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
        sheet_refresh(shtctl, sht_back, 32, 16, 32 + 15 * 8, 32);
        mx += mdec.x;
        my += mdec.y;
        if (mx < 0) {
          mx = 0;
        }
        if (my < 0) {
          my = 0;
        }
        if (mx > binfo->scrnx - 1) {
          mx = binfo->scrnx - 1;
        }
        if (my > binfo->scrny - 1) {
          my = binfo->scrny - 1;
        }
        sprintf(s, "(%d, %d)", mx, my);
        // マウスカーソルを表示し直す処理はそれまでマウスがあった座標を塗りつぶして消して、新しい座標にマウスを書き直す。
        boxfill8(buf_back, binfo->scrnx, COL8_008484, 0, 0, 79, 15); /* 座標消す */
				putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s); /* 座標書く */

        // マウスカーソルが動いたのでリフレッシュ
        sheet_refresh(shtctl, sht_back, 0, 0, 80, 16);
        sheet_slide(shtctl, sht_mouse, mx, my);
      }
    }
  }
}
