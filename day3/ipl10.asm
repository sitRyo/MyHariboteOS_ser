; helloos
; tab=2
; boot sector

CYLS EQU 10 ; CYLS=10と定義する。.data sectionに入れなくてもいいのか

  org 0x7c00

; 標準的なFAT12フォーマットフロッピーディスクのための記述

  jmp entry
  db 0x90
  db "HARIBOTE"
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
  db "HARIBOTEOS" ; 11byte
  db "FAT12   " ; 8byte
  times 18 db 0 ; 18回0を書き込む(resbでも良くない？良くないか...)

; プログラム本体
entry:
  mov ax,0
  mov ss,ax
  mov sp,0x7c00
  mov ds,ax

; ディスクを読む
  mov ax,0x0820 
  mov es,ax
  mov ch,0 ; シリンダ0
  mov dh,0 ; ヘッド0
  mov cl,2 ; セクタ2
readloop:
  mov si,0 ; 失敗回数を数えるレジスタ
retry:
  mov ah,0x02 ; ah=0x02ディスク読み込み(bios命令)
  mov al,1
  mov bx,0
  mov dl,0x00 ; Aドライブ
  int 0x13 ; software intterupt ディスクBIOS呼び出し
  jnc next ; jnc -> cpuのcarry flagが上がっているか?
  add si,1
  cmp si,5
  jae error ; si >= 5だったらerrorへ飛ぶ
  mov ah,0x00
  mov dl,0x00
  int 0x13 ; driveのリセット
  jmp retry
next:
  mov ax,es ; esの内容をaxに移す
  add ax,0x0020 ; アドレスを0x200=512加算
  mov es,ax ; add es, 0x020という命令がないのでこうしている
  add cl,1
  cmp cl,18
  jbe readloop
  mov cl,1
  add dh,1
  cmp dh,2
  jb readloop
  mov dh,0
  add ch,1
  cmp ch,CYLS
  jb readloop

; haribote.sysを実行
  mov [0x0ff0],ch
  jmp 0xc200

error:
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
