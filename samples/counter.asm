start:
	ldi 0x00
	tab
loop:
	ldi 0x00
	tab
	lda count
	cmp 0xff
	jmp exit
	inm count
	ldi 0x00
	tab
	ldi 0x0d
	pos
	num count
	dly 0xff
	lpc loop
count:
	byte 0x00

exit:
	ldi 0x00
	sta count
	byte 0x00 