# 学び, 備忘録
## Day2
* MBR
  * OS読み込み用のプログラムが入っている補助記憶装置の先頭の部分。アドレス的には `0x7c00`。

* `times 0x01fe-($-$$) db 0` の `$-$$` は物理アドレスの先頭から現在のアドレスの差を計算している。
  * 例えば、現在の物理アドレスが0x7cE0だとすると(これが`$`)、`$-$$`は先頭(これが`$$`)アドレスとなる`0x7c00`との差を取る計算となり、差分は `0x00E0`。

* Makefile
  ```makefile
  <label>: <dependencies>
    <command>
  
  <dependencies>: 
    <command>
  
  # make <label> とすると、最初にdependenciesのラベルが実行されて、終了次第、依存元に戻る。
  ```

  example
  ```makefile
  img: 
	nasm helloos.asm

  run: img
	qemu-system-i386 -drive file=helloos,format=raw,if=floppy -boot a
  ```

* nasmのコンパイルオプション
  * -o : 指定したファイル名で出力する。
  * -l : listファイルを出力する。
    * アセンブラが生成したCPUが実行するバイナリコードと対応するアセンブラがリスト形式で表示される。

参考: https://www.mztn.org/lxasm64/amd02.html

## Day4
* VRAM
  * VRAMのメモリマップは0xa0000~0xaffff
  * ここに画面に表示したい色番号を指定していく。
* CLI 
  * ローカルだけど, CLear Interrupt flagの略語
  * 割り込みフラグを0にする(すなわち割り込みができないようにする)
* STI
  * 同様に SeT interrupt flagの略語
  * 割り込みフラグを1にする(すなわち割り込みできるようにする)
* EFLAGS
  * キャリーフラグや割り込みフラグなどが詰まった32bitのレジスタのこと。
  * 本編では割り込みフラグが1である状態を再現するためにEFLAGSを保存し、STIで保存したEFLAGSを使って割り込みフラグが1の状態を復元した。
* ディスプレイとVRAMについて
  * 0xa0000 + x + y * 320

## Day5
* struct BOOTINFO
  |メンバ|size|内容|
  |:---:|:---:|:---:|
  |cyls|char|読み出した位置を保存|
  |leds|char|LED状態を保存(for keyboard?)|
  |vmode|char|画面の状態|
  |reserve|char|?|
  |scrnx|short|画面の大きさ(width)|
  |scrny|short|画面の大きさ(height)|
  |vram|int|vramのメモリマップ(0x000a0000 0 0 0 \| 先頭から0x0ff8 0x0ff9 0x0ffa 0x0ffb)|

参考 http://bttb.s1.valueserver.jp/wordpress/blog/2017/12/17/makeos-5-2/

* -fno-pieオプションについて
自作sprintfをコンパイルするに当たって改めて調べたので書いてみる。
pieとはposition independent executablesの略で日本語だと位置独立実行形式のこと。
pie形式ではPIEバイナリとその依存関係はアプリケーションが実行されるたびに仮想メモリ内のランダムな場所にロードされる。
したがってリターン志向プログラミング(ROP?????)攻撃の実行を妨げる効果がある。

これ以上必要になったらここ読めばよさそう。 https://access.redhat.com/blogs/766093/posts/1975793