img: 
	nasm helloos.asm

run: img
	qemu-system-i386 -drive file=helloos,format=raw,if=floppy -boot a
