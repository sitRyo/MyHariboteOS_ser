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
void set_palette(int start, int end, unsigned char *rgb);

struct BOOTINFO {
  char cyls, leds, vmode, reserve;
  short scrnx, scrny;
  char *vram;
};

void HariMain(void) {
  // from asm
  struct BOOTINFO *binfo;

  binfo = (struct BOOTINFO *) 0x0ff0;

  char *vram;
  int xsize, ysize;

  xsize = binfo->scrnx;
  ysize = binfo->scrny;
  vram = binfo->vram; // vramは0x0ff8から4byte取ってるけど、vramのアドレス自体は先頭1byteだからchar*でOK?(たぶん)

  init_palette();

  boxfill8(vram, xsize, COL8_008484,  0,         0,          xsize -  1, ysize - 29);
	boxfill8(vram, xsize, COL8_C6C6C6,  0,         ysize - 28, xsize -  1, ysize - 28);
	boxfill8(vram, xsize, COL8_FFFFFF,  0,         ysize - 27, xsize -  1, ysize - 27);
	boxfill8(vram, xsize, COL8_C6C6C6,  0,         ysize - 26, xsize -  1, ysize -  1);

	boxfill8(vram, xsize, COL8_FFFFFF,  3,         ysize - 24, 59,         ysize - 24);
	boxfill8(vram, xsize, COL8_FFFFFF,  2,         ysize - 24,  2,         ysize -  4);
	boxfill8(vram, xsize, COL8_848484,  3,         ysize -  4, 59,         ysize -  4);
	boxfill8(vram, xsize, COL8_848484, 59,         ysize - 23, 59,         ysize -  5);
	boxfill8(vram, xsize, COL8_000000,  2,         ysize -  3, 59,         ysize -  3);
	boxfill8(vram, xsize, COL8_000000, 60,         ysize - 24, 60,         ysize -  3);

	boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 24, xsize -  4, ysize - 24);
	boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 23, xsize - 47, ysize -  4);
	boxfill8(vram, xsize, COL8_FFFFFF, xsize - 47, ysize -  3, xsize -  4, ysize -  3);
	boxfill8(vram, xsize, COL8_FFFFFF, xsize -  3, ysize - 24, xsize -  3, ysize -  3);


  while (1) {
    io_hlt();
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