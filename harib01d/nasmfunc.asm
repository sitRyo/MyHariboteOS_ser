; nasmfunc
; tab=4

; for object file
section .text
	GLOBAL	io_hlt, write_mem8

; 実際の関数
io_hlt: 
	HLT
	RET

; write_mem8(int addr, int data)のアドレス
; int main() {
; 	...
;   write_mem8()
;   ...
; }
; みたいになってたら、write_mem8をテキストから読み取る。
write_mem8:
	MOV ECX,[ESP+4]
	MOV AL,[ESP+8]
	MOV [ECX],AL
	RET