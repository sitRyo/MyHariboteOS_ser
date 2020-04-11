CFLAG=-fno-pie -march=i486 -m32 -nostdlib
CC=gcc
OBJBOOTPACK=har.ld keytable.c bootpack.c hankaku.c nasmfunc.o dsctbl.o graphic.o int.o fifo.o mysprintf.o keyboard.o mouse.o memory.o sheet.o timer.o

default:
	make img

ipl10.bin : ipl10.asm Makefile
	nasm ipl10.asm -o ipl10.bin -l ipl10.lst

asmhead.bin : asmhead.asm Makefile
	nasm asmhead.asm -o asmhead.bin -l asmhead.lst

nasmfunc.o: nasmfunc.asm Makefile
	nasm -f elf nasmfunc.asm -o nasmfunc.o -l nasmfunc.lst # -fは出力するファイル形式を指定する。ここではlinuxの実行ファイルフォーマットのelf

# m32つけないと64bit elf形式でコンパイルされてしまうのでbootpack.hrbと実行ファイルの整合性が取れなくなる。
# -c でリンカの処理をスキップ
# -fno-pieで実行ファイルをPIE(Position Independent Executables)にしないように設定する。
# これにより依存関係にあるバイナリを異なる仮想メモリ上で個別に実行することが可能になり、セキュリティが高まるらしい(いまいちわかってない。このオプションを使うことでオブジェクトファイルがどう変わるとか.)。
mysprintf.o: mysprintf.c har.ld Makefile
	$(CC) $(CFLAG) -c mysprintf.c -o mysprintf.o

# Cファイルをリンカスクリプトを用いてコンパイル
# 短縮できない？(*.oだけまとめて記述できるみたいな)
bootpack.hrb : $(OBJBOOTPACK) Makefile
	$(CC) $(CFLAG) -T $(OBJBOOTPACK) -o bootpack.hrb

haribote.sys : asmhead.bin bootpack.hrb Makefile
	cat asmhead.bin bootpack.hrb > haribote.sys

haribote.img : ipl10.bin haribote.sys Makefile
	mformat -f 1440 -C -B ipl10.bin -i haribote.img ::
	mcopy haribote.sys -i haribote.img ::

#### 一般規則
%.o: %.c Makefile
	$(CC) $(CFLAG) -c $*.c -o $*.o

#### コマンド

asm :
	make -r ipl10.bin

img :
	make -r haribote.img

# なんかkvmを使用するといいらしい
run :
	make img
	sudo qemu-system-i386 -m 32 -fda haribote.img -enable-kvm

debug :
	make img
	qemu-system-i386 -fda haribote.img -gdb tcp::10000 -S

clean :
	rm *.lst *.bin *.sys *.img *.hrb *.o 