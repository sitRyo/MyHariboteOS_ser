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
