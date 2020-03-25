// 色設定を予めしておこうという話。
extern void io_hlt(void);
extern void io_cli(void);
extern void io_out8(int port, int data);
extern int io_load_eflags(void);
extern void io_store_eflags(int eflags);

// prototype
void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);

void HariMain(void) {
  int i;
  char* p = (char*) 0xa0000;

  // 色番号 0~15を設定
  init_palette();

  // VRAMのメモリマップは0xa0000から。
  for (i = 0; i <= 0xffff; ++i) {
    *(i + p) = i & 0x9f;
  }

  while (1) {
    io_hlt();
  }
}

void init_palette(void) {
  // DB命令と同じ
  // table_rgbというラベルから3byteずつ色番号を書いていく。
	static unsigned char table_rgb[16 * 3] = {
		0x00, 0x00, 0x00,	/*  0:?? */
		0xff, 0x00, 0x00,	/*  1:?????? */
		0x00, 0xff, 0x00,	/*  2:?????? */
		0xff, 0xff, 0x00,	/*  3:???????F */
		0x00, 0x00, 0xff,	/*  4:?????? */
		0xff, 0x00, 0xff,	/*  5:?????? */
		0x00, 0xff, 0xff,	/*  6:???????F */
		0xff, 0xff, 0xff,	/*  7:?? */
		0xc6, 0xc6, 0xc6,	/*  8:?????D?F */
		0x84, 0x00, 0x00,	/*  9:????? */
		0x00, 0x84, 0x00,	/* 10:????? */
		0x84, 0x84, 0x00,	/* 11:??????F */
		0x00, 0x00, 0x84,	/* 12:????? */
		0x84, 0x00, 0x84,	/* 13:????? */
		0x00, 0x84, 0x84,	/* 14:??????F */
		0x84, 0x84, 0x84	/* 15:????D?F */
	};
  set_palette(0, 15, table_rgb);
  return;
}

void set_palette(int start, int end, unsigned char *rgb) {
  int i,eflags;
  eflags = io_load_eflags();
  io_cli();
  // 0x03c8に色番号を入れることで、この色番号を指定するよって伝える。
  // 0x03c8は装置番号
  io_out8(0x03c8, start);
  for (i = start; i <= end; ++i) {
    io_out8(0x03c9, rgb[0] / 4); // つまり2bit右にシフトしている。
    io_out8(0x03c9, rgb[1] / 4);
    io_out8(0x03c9, rgb[2] / 4);
    rgb += 3;
  }
  // eflagsを復元(割り込みフラグ0 → 1)
  io_store_eflags(eflags);
}