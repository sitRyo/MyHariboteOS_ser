; hlt.asm
; tab=4

; bitsディレクティブなしでhltコマンドを実行するとqemuが再起動する不具合が起こる。
bits 32
    MOV     AL,'A'
    INT     0x40
    RETF