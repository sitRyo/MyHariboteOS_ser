; helloos
; tab=2
; boot sector

  org 0x7c00

; 標準的なFAT12フォーマットフロッピーディスクのための記述

  jmp entry
  db 0x90
  db "HELLOIPL"
  dw 512
  db 1
  dw 1
  db 2
  dw 224
  dw 2880
  db 0xf0
  dw 9
  dw 18
  dw 2
  dd 0
  dd 2880
  db 0,0,0x29
  dd 0xffffffff
  db "HELLO-OS   " ; 11byte
  db "FAT12   " ; 8byte
  times 18 db 0 ; 18回0を書き込む(resbでも良くない？良くないか...)

; プログラム本体
entry:
  mov ax,0
  mov ss,ax
  mov sp,0x7c00
  mov ds,ax
  mov es,ax

  mov si,msg
putloop:
  mov al,[si]
  add si,1
  cmp al,0
  je fin
  mov ah,0x0e
  mov bx,15
  int 0x10
  jmp putloop
fin:
  hlt
  jmp fin

msg:
  db 0x0a, 0x0a
  db "hello, world"
  db 0x0a
  db 0

  times 0x01fe-($-$$) db 0 ; 0x01feはMBRから見た相対アドレス。絶対アドレスは0x7dfe, $$は0x7c00(MBRの先頭)
  ; times 510 db 0
  db 0x55, 0xaa

	DB		0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
	times 4600 db 0
	DB		0xf0, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00
	times 1469432 db 0