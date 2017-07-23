#include <p24Fxxxx.h>
#include <xc.h>
#include "stdint.h"
#include "covar014_lab2b_asmlib_v001.h"
#include <stdio.h>
#include <string.h>

#pragma config POSCMOD = NONE           // Primary Oscillator Select (Primary oscillator disabled)
#pragma config I2C1SEL = PRI            // I2C1 Pin Location Select (Use default SCL1/SDA1 pins)
#pragma config IOL1WAY = OFF            // IOLOCK Protection (IOLOCK may be changed via unlocking seq)
#pragma config OSCIOFNC = ON            // Primary Oscillator Output Function (OSC2/CLKO/RC15 functions as port I/O (RC15))
#pragma config FCKSM = CSECME           // Clock Switching and Monitor (Clock switching is enabled, Fail-Safe Clock Monitor is enabled)
#pragma config FNOSC = FRCPLL           // Oscillator Select (Fast RC Oscillator with PLL module (FRCPLL))
#pragma config SOSCSEL = SOSC           // Sec Oscillator Select (Default Secondary Oscillator (SOSC))
#pragma config WUTSEL = LEG             // Wake-up timer Select (Legacy Wake-up Timer)
#pragma config IESO = ON                // Internal External Switch Over Mode (IESO mode (Two-Speed Start-up) enabled)

// CONFIG1
#pragma config WDTPS = PS32768          // Watchdog Timer Postscaler (1:32,768)
#pragma config FWPSA = PR128            // WDT Prescaler (Prescaler ratio of 1:128)
#pragma config WINDIS = ON              // Watchdog Timer Window (Standard Watchdog Timer enabled,(Windowed-mode is disabled))
#pragma config FWDTEN = OFF             // Watchdog Timer Enable (Watchdog Timer is disabled)
#pragma config ICS = PGx1               // Comm Channel Select (Emulator EMUC1/EMUD1 pins are shared with PGC1/PGD1)
#pragma config GWRP = OFF               // General Code Segment Write Protect (Writes to program memory are allowed)
#pragma config GCP = OFF                // General Code Segment Code Protect (Code protection is disabled)
#pragma config JTAGEN = OFF             // JTAG Port Enable (JTAG port is disabled)

void lcd_cmd(char command){
    I2C2CONbits.SEN = 1;
    while(I2C2CONbits.SEN);
    
    _MI2C2IF = 0;
    I2C2TRN = 0b01111100;
    while(!_MI2C2IF);
    
    _MI2C2IF = 0;
    I2C2TRN = 0b00000000;
    while(!_MI2C2IF);
    
    _MI2C2IF = 0;
    I2C2TRN = command;
    while(!_MI2C2IF);
    
    _MI2C2IF = 0;
    I2C2CONbits.PEN = 1;
    while(I2C2CONbits.PEN);
}

void lcd_printChar(char command) {
    I2C2CONbits.SEN = 1;
    while(I2C2CONbits.SEN);
    
    _MI2C2IF = 0;
    I2C2TRN = 0b01111100;
    while(!_MI2C2IF);
    
    _MI2C2IF = 0;
    I2C2TRN = 0b01000000;
    while(!_MI2C2IF);
    
    _MI2C2IF = 0;
    I2C2TRN = command;
    while(!_MI2C2IF);
    
    _MI2C2IF = 0;
    I2C2CONbits.PEN = 1;
    while(I2C2CONbits.PEN);
}

void setup(){
    CLKDIVbits.RCDIV = 0; //Set RCDIV=1:1 (default 2:1) 32MHz or FCY/2=16M
    AD1PCFG = 0x9fff;      //PortB set digital
    TRISA = 0x0000;
    //CNPU1bits.CN2PUE = 1;//RA0
    I2C2CONbits.I2CEN = 0;
    I2C2BRG = 157;
    _MI2C2IF = 0;
    I2C2CONbits.I2CEN = 1;
    
    T1CON = 0;
    PR1 = 40000;
    T1CONbits.TCKPS0 = 1;
    T1CONbits.TCKPS1 = 1;
    T1CONbits.TON = 1;
    TMR1 = 0;
    _T1IF = 0;
}

void lcd_init(){
    int i;
    for (i=0; i<50; i++){
        delay1ms();
    }
    
    lcd_cmd(0b00111000); // function set, normal instruction mode
    lcd_cmd(0b00111001); // function set, extended instruction mode
    lcd_cmd(0b00010100); // interval osc
    lcd_cmd(0b00011110); // contrast C3-C0
    lcd_cmd(0b01011101); // C5-C4, Ion, Bon
    lcd_cmd(0b01101100); // follower control
    
    for (i=0; i<200; i++){
        delay1ms();
    }
    
    lcd_cmd(0b00111000); // function set, normal instruction mode
    lcd_cmd(0b00001100); // Display On
    lcd_cmd(0b00000001); // Clear Display
 
}




void lcd_setCursor(int x, int y){
    //row y
    //column x
    char com = (char)((0x40*y + x) + 0b10000000);
    lcd_cmd(com);
}

void lcd_printStr(int start_row, int start_column,const char *s){
    lcd_setCursor(start_column, start_row);
    int i = 0;
    for(; i < strlen(s) ; i++){
        lcd_printChar(s[i]);
    }
}

void lcd_clear(){
    lcd_cmd(0b00000001); // Clear Display
    int i;
    for (i=0; i<5; i++){
        delay1ms();
    }
}

void loop(){
    static int i = 0;
    lcd_clear();
    lcd_printStr(0,i,"Hello");
    lcd_printStr(1,i,"World!");
    i++;
    i&=7;
}


int main(void) {
    setup();
    lcd_init();
    while(1){
        loop();
    };
    return 0;
}

