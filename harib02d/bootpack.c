// �����`�������B
#define COL8_000000		0
#define COL8_FF0000		1
#define COL8_00FF00		2
#define COL8_FFFF00		3
#define COL8_0000FF		4
#define COL8_FF00FF		5
#define COL8_00FFFF		6
#define COL8_FFFFFF		7
#define COL8_C6C6C6		8
#define COL8_840000		9
#define COL8_008400		10
#define COL8_848400		11
#define COL8_000084		12
#define COL8_840084		13
#define COL8_008484		14
#define COL8_848484		15

extern void io_hlt(void);
extern void io_cli(void);
extern void io_out8(int port, int data);
extern int io_load_eflags(void);
extern void io_store_eflags(int eflags);
extern void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);

// prototype
void init_palette(void);
void init_screen(char *vram, int x, int y);
void set_palette(int start, int end, unsigned char *rgb);
void putfont8(char *vram, int xsize, int x, int y, char c, char *font);

struct BOOTINFO {
  char cyls, leds, vmode, reserve;
  short scrnx, scrny;
  char *vram;
};

void HariMain(void) {
  // from asm
  struct BOOTINFO *binfo;
  // 文字Aのフォントデータ
  static char font_A[16] = {
    0x00, 0x18, 0x18, 0x18, 0x18, 0x24, 0x24, 0x24,
    0x24, 0x7e, 0x42, 0x42, 0x42, 0xe7, 0x00, 0x00
  };

  binfo = (struct BOOTINFO *) 0x0ff0;

  init_palette();
  init_screen(binfo->vram, binfo->scrnx, binfo->scrny);
  putfont8(binfo->vram, binfo->scrnx, 10, 10, COL8_FFFFFF, font_A);

  while (1) {
    io_hlt();
  }
}

// OS風なスクリーンを作る。
void init_screen(char *vram, int x, int y) {
  boxfill8(vram, x, COL8_008484,  0,         0,          x -  1, y - 29);
	boxfill8(vram, x, COL8_C6C6C6,  0,         y - 28, x -  1, y - 28);
	boxfill8(vram, x, COL8_FFFFFF,  0,         y - 27, x -  1, y - 27);
	boxfill8(vram, x, COL8_C6C6C6,  0,         y - 26, x -  1, y -  1);

	boxfill8(vram, x, COL8_FFFFFF,  3,         y - 24, 59,         y - 24);
	boxfill8(vram, x, COL8_FFFFFF,  2,         y - 24,  2,         y -  4);
	boxfill8(vram, x, COL8_848484,  3,         y -  4, 59,         y -  4);
	boxfill8(vram, x, COL8_848484, 59,         y - 23, 59,         y -  5);
	boxfill8(vram, x, COL8_000000,  2,         y -  3, 59,         y -  3);
	boxfill8(vram, x, COL8_000000, 60,         y - 24, 60,         y -  3);

	boxfill8(vram, x, COL8_848484, x - 47, y - 24, x -  4, y - 24);
	boxfill8(vram, x, COL8_848484, x - 47, y - 23, x - 47, y -  4);
	boxfill8(vram, x, COL8_FFFFFF, x - 47, y -  3, x -  4, y -  3);
	boxfill8(vram, x, COL8_FFFFFF, x -  3, y - 24, x -  3, y -  3);
}

void putfont8(char *vram, int xsize, int x, int y, char c, char *font) {
  int i;
  char *p, d;
  for (i = 0; i < 16; ++i) {
    p = vram + (y + i) * xsize + x;
    d = font[i];
    for (int j = 0; j < 8; ++j) p[j] = (d >> 7 - j) & 1 ? c : COL8_008484;
  }
}

void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1) {
  int x, y;
  for (y = y0; y <= y1; ++y) {
    for (x = x0; x <= x1; ++x) {
      vram[y * xsize + x] = c;
    }
  }
  return;
}

void init_palette(void) {
  // DB���߂Ɠ���
  // table_rgb�Ƃ������x������3byte���F�ԍ��������Ă����B
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
  // 0x03c8�ɐF�ԍ������邱�ƂŁA���̐F�ԍ����w�肷�����ē`����B
  // 0x03c8�͑��u�ԍ�
  io_out8(0x03c8, start);
  for (i = start; i <= end; ++i) {
    io_out8(0x03c9, rgb[0] / 4); // �܂�2bit�E�ɃV�t�g���Ă���B
    io_out8(0x03c9, rgb[1] / 4);
    io_out8(0x03c9, rgb[2] / 4);
    rgb += 3;
  }
  // eflags�𕜌�(���荞�݃t���O0 �� 1)
  io_store_eflags(eflags);
}