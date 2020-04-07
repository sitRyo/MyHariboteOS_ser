// memory管理関係

#include "bootpack.h"

unsigned int memtest(unsigned int start, unsigned int end) {
  char flg486 = 0;
	unsigned int eflg, cr0, i;

  // AC-bitをチェックして386か, 486以降なのかを調査する。
  // 386(キャッシュメモリが存在しない)ではACBITを1にしても自動で0に戻ってしまう。
	eflg = io_load_eflags();

  // EFLAGSのEFLAGS_AC_BITを1にする演算
	eflg |= EFLAGS_AC_BIT; 

  // EFLAGSをもとに戻す。
	io_store_eflags(eflg);

  // 保存したEFLAGS(AC-bitを1にしたEFLAGS)を読み出す。
	eflg = io_load_eflags();

  // CPUが386だったら0になっている
	if ((eflg & EFLAGS_AC_BIT) != 0) { 
		flg486 = 1;
	}

  // AC-bitを0にもどす
	eflg &= ~EFLAGS_AC_BIT;
	io_store_eflags(eflg);

	if (flg486 != 0) {
		cr0 = load_cr0();
		cr0 |= CR0_CACHE_DISABLE; 
		store_cr0(cr0);
	}

	i = memtest_sub(start, end);

	if (flg486 != 0) {
		cr0 = load_cr0();
		cr0 &= ~CR0_CACHE_DISABLE; 
		store_cr0(cr0);
	}

	return i;
}

// メモリマネージャの要素を初期化
void memman_init(struct MEMMAN *man) {
  man->frees = 0; // 空き情報の個数
  man->maxfrees = 0; // 状況観察用
  man->lostsize = 0; // 解放に失敗した合計サイズ
  man->losts = 0; // 解放に失敗した回数
}

// 空きサイズの合計を報告
unsigned int memman_total(struct MEMMAN *man) {
	unsigned int i, t = 0;
	for (i = 0; i < man->frees; i++) {
		t += man->free[i].size;
	}
	return t;
}

// メモリの確保を行う。
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size) {
  unsigned int i, a;
  for (i = 0; i < man->frees; i++) {
		if (man->free[i].size >= size) {
			/* 十分な広さのあきを発見 */
			a = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;
			if (man->free[i].size == 0) {
				/* free[i]がなくなったので前へつめる */
				man->frees--;
				for (; i < man->frees; i++) {
					man->free[i] = man->free[i + 1]; /* 構造体の代入 */
				}
			}
			return a;
		}
	}
	return 0; /* あきがない */
}

// メモリを解放する。
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size) {
	int i, j;
	/* まとめやすさを考えると、free[]がaddr順に並んでいるほうがいい */
	/* だからまず、どこに入れるべきかを決める */
	for (i = 0; i < man->frees; i++) {
		if (man->free[i].addr > addr) {
			break;
		}
	}
	/* free[i - 1].addr < addr < free[i].addr */
	if (i > 0) {
		/* 前がある */
		if (man->free[i - 1].addr + man->free[i - 1].size == addr) {
			/* 前のあき領域にまとめられる */
			man->free[i - 1].size += size;
			if (i < man->frees) {
				/* 後ろもある */
				if (addr + size == man->free[i].addr) {
					/* なんと後ろともまとめられる */
					man->free[i - 1].size += man->free[i].size;
					/* man->free[i]の削除 */
					/* free[i]がなくなったので前へつめる */
					man->frees--;
					for (; i < man->frees; i++) {
						man->free[i] = man->free[i + 1]; /* 構造体の代入 */
					}
				}
			}
			return 0; /* 成功終了 */
		}
	}
	/* 前とはまとめられなかった */
	if (i < man->frees) {
		/* 後ろがある */
		if (addr + size == man->free[i].addr) {
			/* 後ろとはまとめられる */
			man->free[i].addr = addr;
			man->free[i].size += size;
			return 0; /* 成功終了 */
		}
	}
	/* 前にも後ろにもまとめられない */
	if (man->frees < MEMMAN_FREES) {
		/* free[i]より後ろを、後ろへずらして、すきまを作る */
		for (j = man->frees; j > i; j--) {
			man->free[j] = man->free[j - 1];
		}
		man->frees++;
		if (man->maxfrees < man->frees) {
			man->maxfrees = man->frees; /* 最大値を更新 */
		}
		man->free[i].addr = addr;
		man->free[i].size = size;
		return 0; /* 成功終了 */
	}
	/* 後ろにずらせなかった */
	man->losts++;
	man->lostsize += size;
	return -1; /* 失敗終了 */
}

// sizeに必要な容量を指定するとそれに見合った4kの倍数のサイズを持つ容量を取得する。
unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size) {
  unsigned int a;
  // 切り捨て処理。
  // 4k=4096byte. 4097byteの容量が必要になったら8kbyte必要だが, floor(4097 + 4k-1)で8kbyteが取れる。
  size = (size + 0xfff) & 0xfffff000;
  // return address
  a = memman_alloc(man, size);
  return a;
}

int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size) {
  int i;
  // sizeからこのメモリのために確保している容量を計算する。
  size = (size + 0xfff) & 0xfffff000;
  i = memman_free(man, addr, size);
  return i;
}