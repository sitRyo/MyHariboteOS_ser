// timer.c

#include "bootpack.h"

#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040

struct TIMERCTL timerctl;

void init_pit(void) {
  io_out8(PIT_CTRL, 0x34);
  // タイマ割り込み周期設定
  io_out8(PIT_CNT0, 0x9c); // 下位8bit
  io_out8(PIT_CNT0, 0x2e); // 上位8bit
  timerctl.count = 0;
}

void inthandler20(int *esp) {
	io_out8(PIC0_OCW2, 0x60); // 割り込み受け付け完了をPICに通知。
  timerctl.count += 1;
  if (timerctl.timeout > 0) {
    timerctl.timeout--;
    if (timerctl.timeout == 0) {
      fifo8_put(timerctl.fifo, timerctl.data);
    }
  }
}

void settimer(unsigned int timeout, struct FIFO8 *fifo, unsigned char data) {
	int eflags;
	eflags = io_load_eflags();
	io_cli();
	timerctl.timeout = timeout;
	timerctl.fifo = fifo;
	timerctl.data = data;
	io_store_eflags(eflags);
}