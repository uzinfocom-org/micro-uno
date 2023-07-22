start:
  ldi 0x00
  tab

print:
  lda hello
  cmp 0x00
  jmp exit
  dly 0xff
  out
  inc
  lpc print

hello:
  byte 0x53
  byte 0x61
  byte 0x6C
  byte 0x6F
  byte 0x6D
  byte 0x20
  byte 0x64
  byte 0x75
  byte 0x6E
  byte 0x79
  byte 0x6F
  byte 0x21

hello_end:
  byte 0x00

exit:
  ldi 0x01
  tab
  ldi 0x00
  pos
  byte 0x00