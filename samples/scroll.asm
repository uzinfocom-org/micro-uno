start:
  ncr
  ldi 0x00
  tab
  ldi 0x10 
  pos
  ldi 0x21
  
loop:
  add 0x01
  out
  sdl
  dly 0xff
  dly 0xff
  lpc loop
loop_end:
  byte 0x00
exit:
  byte 0x00