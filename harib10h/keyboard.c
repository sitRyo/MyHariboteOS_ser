#include "bootpack.h"

struct FIFO32 *keyfifo;
int keydata0;

// Keyboard
void inthandler21(int *esp) {
	int data;
	io_out8(PIC0_OCW2, 0x61); // IRQ-01受け付け完了をPICに通知。
	data = io_in8(PORT_KEYDAT);
	fifo32_put(keyfifo, data + keydata0);
}

// keyboard制御回路(キーボードコントローラ:KBC)が制御命令を受け付けられるようになるまで待つ。
void wait_KBC_sendready(void) {
  for(;;) {
    // 命令を受け付けられるようになると, 0x0064の下から2bitが0になる。
    if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
      break;
    }
  }
}

void init_keyboard(struct FIFO32 *fifo, int data0) {
  keyfifo = fifo;
  keydata0 = data0;

  wait_KBC_sendready();
  // モード設定
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
  // Mouseの使用設定が0x47
	io_out8(PORT_KEYDAT, KBC_MODE);
	return;
}

