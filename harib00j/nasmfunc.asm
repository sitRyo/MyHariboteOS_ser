; nasmfunc
; tab=4

; for object file
section .text
	GLOBAL	_io_hlt

; 実際の関数
_io_hlt: 
	HLT
	RET
