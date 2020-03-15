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