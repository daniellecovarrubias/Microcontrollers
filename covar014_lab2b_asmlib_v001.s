.equ __P24FJ64GA002,1 ;required "boilerplate" (BP)
.include "p24Fxxxx.inc" ;BP
#include "xc.inc" ;BP
.text ;BP (put the following data in ROM(program memory))
; This is a library, thus it can *not* contain a _main function: the C file will
; deine main(). However, we
; we will need a .global statement to make available ASM functions to C code.
; All functions utilized outside of this file will need to have a leading
; underscore (_) and be included in a comment delimited list below.

.global _write_0, _write_1, _delay50us, _delay1ms
    
_write_0:
    ;2 low
    inc LATA
    repeat #4 ;1 high
    nop ;1+4=5 high
    clr LATA
    repeat #7 ;1 low
    nop ;1+7=8 low
    return ;3 low

_write_1:
    ;2 low
    inc LATA
    repeat #9 ;1 high
    nop ;1+9=10 high
    clr LATA
    repeat #2 ;1 low
    nop ;1+2=3 low
    return ;3 low

_delay50us:
	repeat #793
	nop
	return
	
_delay1ms:
    repeat #15993
    nop
    return