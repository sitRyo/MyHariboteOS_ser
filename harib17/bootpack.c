/**
 * hairb16 commandline
 */ 

#include "bootpack.h"

void HariMain(void) {
	extern char keytable0[0x80], keytable1[0x80];
  struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	char s[40];
	int mx, my, i;
	unsigned int memtotal;
	struct MOUSE_DEC mdec;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;
	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse, *sht_win, *sht_cons;
	unsigned char *buf_back, buf_mouse[256], *buf_win, *buf_cons;
	struct FIFO32 fifo;
	int fifobuf[128];
	struct TIMER *timer, *timer2, *timer3, *timer_ts;
	int cursor_x, cursor_c, task_b_esp;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	struct TASK *task_a, *task_cons;
	int key_to = 0, key_shift = 0, key_leds = (binfo->leds >> 4) & 7;

  // gdt(Segment Descriptor), idt(Inttrupt Descriptor)設定。
  // segment 2(だったはず)にidtを保存
	init_gdtidt();

  // picを初期化(HWからの割り込み初期化)
	init_pic();

  /* IDT/PICの初期化が終わったのでCPUの割り込み禁止を解除 */
  // Inttruptフラグを1に。
	io_sti();
	fifo32_init(&fifo, 128, fifobuf, 0);
	init_pit(); // timer初期化
	init_keyboard(&fifo, 256);
	enable_mouse(&fifo, 512, &mdec);
	io_out8(PIC0_IMR, 0xf8); /* PIC1とキーボードを許可(11111001) */
	io_out8(PIC1_IMR, 0xef); /* マウスを許可(11101111) */

	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
	memman_free(memman, 0x00400000, memtotal - 0x00400000);

	// 色情報・vram設定
	init_palette();
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	task_a = task_init(memman);
	fifo.task = task_a;
	task_run(task_a, 1, 2);

	// sht_back
	sht_back = sheet_alloc(shtctl);
	buf_back = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
	init_screen(buf_back, binfo->scrnx, binfo->scrny);

	// task_console
	sht_cons = sheet_alloc(shtctl);
	buf_cons = (unsigned char *) memman_alloc_4k(memman, 256 * 165);
	sheet_setbuf(sht_cons, buf_cons, 256, 165, -1); /* 透明色なし */
	make_window8(buf_cons, 256, 165, "console", 0);
	make_textbox8(sht_cons, 8, 28, 240, 128, COL8_000000);
	task_cons = task_alloc();
	task_cons->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 12;
	task_cons->tss.eip = (int) &console_task;
	task_cons->tss.es = 1 * 8;
	task_cons->tss.cs = 2 * 8;
	task_cons->tss.ss = 1 * 8;
	task_cons->tss.ds = 1 * 8;
	task_cons->tss.fs = 1 * 8;
	task_cons->tss.gs = 1 * 8;
	*((int *) (task_cons->tss.esp + 4)) = (int) sht_cons;
	*((int *) (task_cons->tss.esp + 8)) = memtotal;
	task_run(task_cons, 2, 2); /* level=2, priority=2 */

	// sht_win
	sht_win = sheet_alloc(shtctl);
	buf_win = (unsigned char *) memman_alloc_4k(memman, 160 * 52);
	sheet_setbuf(sht_win, buf_win, 144, 52, -1);
	make_window8(buf_win, 144, 52, "task_a", 1);
	make_textbox8(sht_win, 8, 28, 128, 16, COL8_FFFFFF);
	cursor_x = 8;
	cursor_c = COL8_FFFFFF;
	timer = timer_alloc();
	timer_init(timer, &fifo, 1);
	timer_settime(timer, 50);

	/* sht_mouse */
	sht_mouse = sheet_alloc(shtctl);
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	init_mouse_cursor8(buf_mouse, 99);
	mx = (binfo->scrnx - 16) / 2; 
	my = (binfo->scrny - 28 - 16) / 2;

	sheet_slide(sht_back, 0, 0);
	sheet_slide(sht_win, 64, 56);
	sheet_slide(sht_mouse, mx, my);
	sheet_slide(sht_cons, 32, 4);
	sheet_updown(sht_back,  0);
	sheet_updown(sht_cons,  1);
	sheet_updown(sht_win,   2);
	sheet_updown(sht_mouse, 3);

	for (;;) {	
		io_cli();
		if (fifo32_status(&fifo) == 0) {
			task_sleep(task_a);
			io_sti();
		} else {
			i = fifo32_get(&fifo);
			io_sti();
			if (256 <= i && i <= 511) { /* キーボードデータ */
				// keycodeを実行
				if (i < 0x80 + 256) { 
					if (key_shift == 0) {
						s[0] = keytable0[i - 256];
					} else {
						s[0] = keytable1[i - 256];
					}
				} else {
					s[0] = 0;
				}
				if ('A' <= s[0] && s[0] <= 'Z') {
					if (((key_leds & 4) == 0 && key_shift == 0) || ((key_leds & 4) != 0 && key_shift != 0)) {
						s[0] += 0x20;
					}
				}
				// キーコードを入力した
				if (s[0] != 0) { 
					if (key_to == 0) {	/* タスクAへ */
						if (cursor_x < 128) {
							/* 一文字表示してから、カーソルを1つ進める */
							s[1] = 0;
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, s, 1);
							cursor_x += 8;
						}
					} else {	/* コンソールへ */
						fifo32_put(&task_cons->fifo, s[0] + 256);
					}
				}
				if (i == 256 + 0x0e) {	/* バックスペース */
					if (key_to == 0) {	/* タスクAへ */
						if (cursor_x > 8) {
							/* カーソルをスペースで消してから、カーソルを1つ戻す */
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
							cursor_x -= 8;
						}
					} else {	/* コンソールへ */
						fifo32_put(&task_cons->fifo, 8 + 256);
					}
				}
				// Tab文字出力
				if (i == 256 + 0x0f) {
					if (key_to == 0) {
						key_to = 1;
						make_wtitle8(buf_win,  sht_win->bxsize,  "task_a",  0);
						make_wtitle8(buf_cons, sht_cons->bxsize, "console", 1);
						cursor_c = -1;
						boxfill8(sht_win->buf, sht_win->bxsize, COL8_FFFFFF, cursor_x, 28, cursor_x + 7, 43);
						fifo32_put(&task_cons->fifo, 2);
					} else {
						key_to = 0;
						make_wtitle8(buf_win,  sht_win->bxsize,  "task_a",  1);
						make_wtitle8(buf_cons, sht_cons->bxsize, "console", 0);
						cursor_c = COL8_000000;
						fifo32_put(&task_cons->fifo, 3);
					}
					sheet_refresh(sht_win,  0, 0, sht_win->bxsize,  21);
					sheet_refresh(sht_cons, 0, 0, sht_cons->bxsize, 21);
				}
				// Enter
				if (i == 256 + 0x1c) {
					if (key_to != 0) {
						fifo32_put(&task_cons->fifo, 10 + 256);
					}
				}
				if (i == 256 + 0x2a) {	/* 左シフト ON */
					key_shift |= 1;
				}
				if (i == 256 + 0x36) {	/* 右シフト ON */
					key_shift |= 2;
				}
				if (i == 256 + 0xaa) {	/* 左シフト OFF */
					key_shift &= ~1; // 11111110
				}
				if (i == 256 + 0xb6) {	/* 右シフト OFF */
					key_shift &= ~2; // 11111101
				}
				/* カーソルの再表示 */
				if (cursor_c >= 0) {
					boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
				}
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
			} else if (512 <= i && i <= 767) { /* マウスデータ */
				if (mouse_decode(&mdec, i - 512) != 0) {
					/* マウスカーソルの移動 */
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
					sheet_slide(sht_mouse, mx, my);
					if ((mdec.btn & 0x01) != 0) {
						sheet_slide(sht_win, mx - 80, my - 8);
					}
				}
			} else if (i <= 1) {
				if (i != 0) {
					timer_init(timer, &fifo, 0);
					if (cursor_c >= 0) {
						cursor_c = COL8_000000;
					}
				} else {
					timer_init(timer, &fifo, 1);
					if (cursor_c >= 0) {
						cursor_c = COL8_FFFFFF;
					}
				}
				timer_settime(timer, 50);
				if (cursor_c >= 0) {
					boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
					sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
				}
			}
		}
	}
}
