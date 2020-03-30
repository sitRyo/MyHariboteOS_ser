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

* GDT, IDT
  * GDTはセグメンテーションを参照テーブル。IDTは割り込みされた際にどのセグメントを実行するかを指定するテーブル。

## Day6
前日はGDT(global (segment) descriptor table), IDT(interrupt descriptor table)の初期化まで実装

* Makefileの一般規則、生成規則
  * 生成規則はラベルが具体的なもの。
  * 一般規則は生成規則が存在しないものに対して行う。
  ```Makefile
  %.o: %.c Makefile # % はラベル
    $(CC) $(CFLAG) $*.c -o $*.o # $*はターゲットのプレフィックス
  ```
* GDTR
  * 指定されたリミットと番地を格納するレジスタ
    * LGDT(asm)命令によって指定したアドレスから6byteのデータをGDTRに格納する。
* SEGMENT DESCRIPTOR
  * 64bit(8byteの構造体)　下位ビットから見て配置は以下の通り

  |メンバ|size|内容|
  |:---:|:---:|:---:|
  |limit_low|short|リミットの下位16bit|
  |base_low|short|セグメントの開始アドレス下位16bit|
  |base_mid|char|セグメントの開始アドレス中位8bit|
  |access right|char|アクセス権限8bit|
  |limit_high|4bit|リミットの上位4bit|
  |extend access right|4bit|拡張アクセス権限(GD00,と解釈される。Gはこのセグメントをページとして扱うか？Dは16bitor32bitモードかを決定する。)|
  |base_high|char|セグメントの開始アドレス上位8bit|
  
  * limit => 20bit, base address => 32bit, access right => 12bit が分割されてglobal descriptor tableに入っている。すごくわかりづらい。

* PIC
  * programmable interruput controllerの略。
  * 外部装置のアクセス場所を設定する。
  * IMR
    * IMRはPICのレジスタで割り込み目隠しレジスタという名前。
    * レジスタの容量は1byte(=8bit)でそれぞれのbitはIRQに対応し、1のときは信号を無視する(=割り込みを受け付けない)。
  * ICW
    * 初期化制御データ。それぞれ8bitのレジスタ。
    * ICW3はマスタースレーブ接続に関する設定。何番のIRQにスレーブがつながっているかを8bitで設定する。

## Day7
前回はSegment Descriptorの設定を行い、キーボードとマウス用の割り込みハンドラを設定した。
割り込み先は2つ目のセグメントでbootpack.hrbが存在する区画。

* IRQ受け付け完了命令
  * IRQからの割り込みを受け付けたことをPICに通知する命令。PICのPIC0_OCW2レジスタに0x60+IRQ番号を通知する。
  * これを行わないとPICはCPUが割り込みを処理してくれるまで待つことになり、新しい割り込みを行わなくなってしまう。
  * その後に0x0060から8bitデータを入力する。
    * io_in8(PORT_KEYDAT(0x60))

* 割り込み
  * Keyboard
    * Keyboardを押すと、PICがCPUに対して割り込みを入れる。
    * このとき割り込みフラグが1のときは割り込みでき、そうでないときは割り込みできない。
    * PICではIWQによってCPUに直接、呼び出す割り込みハンドラを実行させる。
    * 割り込みハンドラはPICに受信したことを報告し、KeyCodeを受け取ったら割り込みフラグを再び1にする。
    * 割り込みフラグを1にするのはKeycodeをPICから受け取る間だけでいい。
    * また、全てのPIC,基盤で同じ仕様かはわからないが1byteのCodeに対して1回割り込みが起こる。

* マウスの制御回路
  * マウスは起動時には有効になっていないので、キーボード制御回路を介して有効化する必要がある。
  
