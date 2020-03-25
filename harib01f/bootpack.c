// �F�ݒ��\�߂��Ă������Ƃ����b�B
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

  // �F�ԍ� 0~15��ݒ�
  init_palette();

  // VRAM�̃������}�b�v��0xa0000����B
  for (i = 0; i <= 0xffff; ++i) {
    *(i + p) = i & 0x9f;
  }

  while (1) {
    io_hlt();
  }
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