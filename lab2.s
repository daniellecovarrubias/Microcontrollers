.equ __P24FJ64GA002,1       ;required "boiler-plate" (BP)
.include "p24Fxxxx.inc"     ;BP
    
#include "xc.inc"           ;BP

;the next two lines set up the actual chip for operation - required
config __CONFIG2, POSCMOD_EC & I2C1SEL_SEC & IOL1WAY_OFF & OSCIOFNC_ON & FCKSM_CSECME & FNOSC_FRCPLL & SOSCSEL_LPSOSC & WUTSEL_FST & IESO_OFF
config __CONFIG1, WDTPS_PS1 & FWPSA_PR32 & WINDIS_OFF & FWDTEN_OFF & COE_ON & BKBUG_ON & GWRP_ON & GCP_ON & JTAGEN_OFF

	.bss ;put the following labels in RAM

counter:
	.space 2    ; a variable that takes two bytes (we won?t use
		    ; it for now, but put here to make this a generic
		    ;template to be used later).
stack:
	.space 32   ;this will be our stack area, needed for func calls
.text		    ;BP (put the following data in ROM(program memory))
;because we are using the C compiler to assemble our code, we need a "_main" label
;somewhere. (There's a link step that looks for it.)
.global _main ;BP
_main:
    bclr CLKDIV,#8 ;BP
    nop
    ; --- Begin your program below here ---
    mov #0x9fff,w0
    mov w0,AD1PCFG ; Set all pins to digital mode
    mov #0b1111111111111110,w0
    mov w0,TRISA ; set pin RA0 to output
    mov #0x0001,w0
    mov w0,LATA ; set pin RA0 high
    
    wait_10cycles:
			    ; 2 cycles for function call
	repeat #25	    ; 1 cycle to load and prep
	nop		    ; 3+1 cycles to execute NOP 4 times
	return		    ; 3 cycles for the return
    
    ;foreverLoop:
    ;call wait_10cycles
    ;clr LATA		; set pin RA0 low = 1 cycle
    
    ;nop			; 2 cycles to match BRA delay
    ;nop			;
    ;repeat #22		; 1 cycle to load and prep
    ;nop			; 8+1 cycles to execute NOP 9 times
    ;inc LATA		; set pin RA0 high = 1 cycle
			; Total = 12 cycles high, 12 cycles low
    ;bra foreverLoop
    ;.end
   
    
   ; repeat #16
   ; repeat #160
   ; repeat #1600
   ; repeat #16000





