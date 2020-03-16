RM=rm

default :
	make img

##
# nasm option
# -o 出力ファイルの指定
# -l lstファイルの指定, これでアセンブラと機械語の対応が見れる.
##

ipl.bin : ipl10.asm Makefile
	nasm ipl10.asm -o ipl.bin -l ipl.lst

helloos.img: ipl.bin Makefile
	cat ipl.bin > helloos.img

img: ipl.bin Makefile
	make -r helloos.img

asm:
	make -r ipl.bin

run:
	make img
	qemu-system-i386 -drive file=helloos.img,format=raw,if=floppy -boot a

# commandとか大体宣言するよね
clean:
	${RM} ipl.bin
	${RM} ipl.lst