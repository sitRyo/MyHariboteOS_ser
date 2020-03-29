// グラフィック関係
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

// cannot use stdio.h/sprintf
extern void sprintf (char *str, char *fmt, ...);

// prototype
void init_palette(void);
void init_screen(char *vram, int x, int y);
void set_palette(int start, int end, unsigned char *rgb);
void putfont8(char *vram, int xsize, int x, int y, char c, char *font);
void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s);
void init_mouse_cursor8(char *mouse, char bc);
void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char *buf, int bxsize);

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

void putfonts8_asc(char *vram, int xsize, int x, int y, char c, unsigned char *s) {
  extern char hankaku[4096];
  // null文字終端にしないと延々とメモリをcで埋めてしまう。
  while (*s != 0x00) {
    putfont8(vram, xsize, x, y, c, hankaku + *s * 16);
    ++s;
    x += 8;
  }
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

void init_mouse_cursor8(char *mouse, char bc) {
  static char cursor[16][16] = {
		"**************..",
		"*OOOOOOOOOOO*...",
		"*OOOOOOOOOO*....",
		"*OOOOOOOOO*.....",
		"*OOOOOOOO*......",
		"*OOOOOOO*.......",
		"*OOOOOOO*.......",
		"*OOOOOOOO*......",
		"*OOOO**OOO*.....",
		"*OOO*..*OOO*....",
		"*OO*....*OOO*...",
		"*O*......*OOO*..",
		"**........*OOO*.",
		"*..........*OOO*",
		"............*OO*",
		".............***"
	};

  for (int y = 0; y < 16; ++y) {
    for (int x = 0; x < 16; ++x) {
      if (cursor[y][x] == '*') {
        mouse[y * 16 + x] = COL8_000000;
      }
      if (cursor[y][x] == 'O') {
				mouse[y * 16 + x] = COL8_FFFFFF;
			}
			if (cursor[y][x] == '.') {
				mouse[y * 16 + x] = bc;
			}
    }
  }
  return ;
}

void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char *buf, int bxsize) {
	int x, y;
	for (y = 0; y < pysize; y++) {
		for (x = 0; x < pxsize; x++) {
			vram[(py0 + y) * vxsize + (px0 + x)] = buf[y * bxsize + x];
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
