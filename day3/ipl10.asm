; haribote-ipl
; tab=2

  org 0x7c00 
  ; org命令 このプログラムがどこに読み込まれるかを指定する。
  ; 0x7c00 は補助記憶装置の先頭にある特殊な領域。コンピュータが起動する一番最初に読み込まれる部分。

  jmp entry
  db 0x90 
  db "HARIBOTE" ; ブートセクタの名前
  dw 512 ; 1セクタの大きさ
  db 1 ; クラスタの大きさ
  dw 1 ; FATがどこから始まるか
  db 2 ; FATの個数
  dw 224 ; ルートディレクトリ領域の大きさ
  dw 2880 ; このドライブの大きさ (2880にしなければいけない)
  db 0xf0 ; メディアのタイプ
  dw 9 ; FAT領域の長さ
  dw 18 ; 1トラックにいくつのセクタがあるか
  dw 2 ; ヘッドの数
  dd 0 ; パーティションを使っていないので必ず0にする
  dd 2880 ; このドライブの大きさをもう1度書く
  db 0,0,0x29 ; よくわからないらしい
  dd 0xffffffff ; シリアルナンバー?
  db "HARIBOTEOS " ; ディスクの名前
  db "FAT12   " ; フォーマットの名前
  times 18 db 0 ; 18byte開けておく。

entry:
  mov ax,0 ; レジスタ初期化
  mov ss,ax ; ssは直接数値を代入できないので、慣習でaxを介して初期化している。
  mov sp,0x7c00 ; スタックポインタを0x7c00に
  mov ds,ax ; 0に初期化ds:なので
  mov es,ax

  mov si,msg ; siにmsgに入れる。
putloop:
  mov al,[si] ; int 0x0eではalに表示したい文字を入れる
  add si,1 ; siをインクリメント. msgのポインタが1個ずれる
  cmp al,0 ; null文字にあったら終わり
  je fin
  mov ah,0x0e ; 1文字表示(int) 
  mov bx,15 ; color code
  int 0x10 ; 割り込み
  jmp putloop
msg:
  db 0x0a, 0x0a ; 改行を2つ
  db "hello, world!" ; メッセージ
  db 0x0a
  db 0 ; null文字
  
  times 0x01fe - ($-$$) db 0 

  db 0x55, 0xaa ; boot sectorであることを保証する
fin:
  hlt
  jmp fin
