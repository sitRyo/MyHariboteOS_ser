#include "bootpack.h"

void fifo32_init(struct FIFO32 *fifo, int size, int *buf) {
	fifo->buf = buf;
	fifo->size = size;
	fifo->free = size;
	fifo->p = 0; // 書き込み位置
	fifo->q = 0; // 読み出し位置
	fifo->flags = 0; // 何に使うんだっけこれ
}

int fifo32_put(struct FIFO32 *fifo, int data) {
	if (fifo->free == 0) {
		fifo->flags |= FLAGS_OVERRUN;
		return -1;
	}
	fifo->buf[fifo->p] = data;
	fifo->p++;
	if (fifo->p == fifo->size) {
		fifo->p = 0;
	}
	fifo->free--;
	return 0;
}

int fifo32_get(struct FIFO32 *fifo) {
	int data;
	if (fifo->free == fifo->size) {
		return -1;
	}
	data = fifo->buf[fifo->q];
	fifo->q++;
	if (fifo->q == fifo->size) {
		fifo->q = 0;
	}
	fifo->free++;
	return data;
}

int fifo32_status(struct FIFO32 *fifo) {
	return fifo->size - fifo->free;
}