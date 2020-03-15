; nasmだけどファイル名はnask

section .text
  GLOBAL _io_hlt

_io_hlt: ; void io_hlt(void);
  hlt
  ret