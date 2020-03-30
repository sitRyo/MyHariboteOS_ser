// 割り込み関係

#include "bootpack.h"

void init_pic(void)
/* PICの初期化 */
{
  // IMRはPICのレジスタで割り込み目隠しレジスタという名前。
  // レジスタの容量は1byte(=8bit)でそれぞれのbitはIRQに対応し、1のときは信号を無視する(=割り込みを受け付けない)。
	io_out8(PIC0_IMR,  0xff  ); // 割り込みを受け付けないようにする。(0)
	io_out8(PIC1_IMR,  0xff  ); // 割り込みを受け付けないようにする。(1)
  
  // ICWは初期化制御データ。それぞれ8bitのレジスタ。
  // ICW3はマスタースレーブ接続に関する設定。何番のIRQにスレーブがつながっているかを8bitで設定する。
	io_out8(PIC0_ICW1, 0x11  ); 
	io_out8(PIC0_ICW2, 0x20  ); 
	io_out8(PIC0_ICW3, 1 << 2); 
	io_out8(PIC0_ICW4, 0x01  ); 

	io_out8(PIC1_ICW1, 0x11  ); 
	io_out8(PIC1_ICW2, 0x28  ); 
	io_out8(PIC1_ICW3, 2     ); 
	io_out8(PIC1_ICW4, 0x01  ); 

	io_out8(PIC0_IMR,  0xfb  ); 
	io_out8(PIC1_IMR,  0xff  ); 

	return;
}

struct KEYBUF keybuf;

// Keyboard
void inthandler21(int *esp) {
	struct BOOTINFO *binfo = (struct BOOTINFO*) ADR_BOOTINFO;
	unsigned char data, s[4];
	io_out8(PIC0_OCW2, 0x61); // IRQ-01受け付け完了をPICに通知。
	data = io_in8(PORT_KEYDAT);

	// もしkeybufに文字がないなら
	if (keybuf.flag == 0) {
		keybuf.data = data;
		keybuf.flag = 1;
	}
}

void inthandler2c(int *esp) {
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	boxfill8(binfo->vram, binfo->scrnx, COL8_000000, 0, 0, 32 * 8 - 1, 15);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, "INT 2C (IRQ-12) : PS/2 mouse");
	for (;;) {
		io_hlt();
	}
}

void inthandler27(int *esp) {
	io_out8(PIC0_OCW2, 0x67);
	return;
}
