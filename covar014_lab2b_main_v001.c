/*
 * File:   covar014_lab2b_main_v001.c
 * Author: daniellecovarrubias
 *
 * Created on February 5, 2017, 4:29 PM
 */

#include "covar014_lab2b_asmlib_v001.h"
#include "xc.h"
#include <p24Fxxxx.h>
#include "stdint.h"

#define FRAME_PER 10 //Period (1/FrameRate) in milliseconds

//CW1: FLASH CONFIGURATION WORD 1 (see PIC24 Family Reference Manual 24.1)
#pragma config ICS = PGx1 // Comm Channel Select (Emulator EMUC1/EMUD1 pins are shared with PGC1/PGD1)
#pragma config FWDTEN = OFF // Watchdog Timer Enable (Watchdog Timer is disabled)
#pragma config GWRP = OFF // General Code Segment Write Protect (Writes to program memory are allowed)
#pragma config GCP = OFF // General Code Segment Code Protect (Code protection is disabled)
#pragma config JTAGEN = OFF // JTAG Port Enable (JTAG port is disabled)

// CW2: FLASH CONFIGURATION WORD 2 (see PIC24 Family Reference Manual 24.1)
#pragma config I2C1SEL = PRI // I2C1 Pin Location Select (Use default SCL1/SDA1 pins)
#pragma config IOL1WAY = OFF // IOLOCK Protection (IOLOCK may be changed via unlocking seq)
#pragma config OSCIOFNC = OFF // Primary Oscillator I/O Function (CLKO/RC15 functions as I/O pin)
#pragma config FCKSM = CSECME // Clock Switching and Monitor (Clock switching is enabled, Fail-Safe Clock Monitor is enabled)
#pragma config FNOSC = FRCPLL // Oscillator Select (Fast RC Oscillator with PLL module (FRCPLL))


void setup(void)
    {
        //CLKDIVbits.RCDIV = 0; //Set RCDIV=1:1 (default 2:1) 32MHz or FCY/2=16M
        asm("bclr CLKDIV,#8");
        

        AD1PCFG = 0x9fff;
        TRISA = 0b1111111111111110;
        LATA = 0x0000;
    }

/*void drawFrame(int frame){
    int red=frame;
    int blue= 255-frame;
    writeColor(red,0,blue);
}*/



void delay(int delay_in_ms){
    while (delay_in_ms>=0){
        delay1ms();
        delay_in_ms--;
    }
}

uint32_t packColor(int Red, int Blu, int Grn) {
    unsigned long int RGBval=0;
    RGBval = ((long) Red << 16) | ((long) Grn << 8) | ((long) Blu);
    return RGBval;
}

int getR(uint32_t RGBval){
    int Red=0;
    Red = (int) (RGBval >> 16);
    return Red;
}

int getG(uint32_t RGBval){
    int Green=0;
    Green = (int) (RGBval >> 8);
    return Green;
}

int getB(uint32_t RGBval){
    int Blue=0;
    Blue = (int) (RGBval >> 0);
    return Blue;
}

void writePacCol(uint32_t PackedColor){
    writeColor(getR(PackedColor), getG(PackedColor), getB(PackedColor));
}

void writeColor(int r, int g, int b){
    //red
    int i; 
    for (i=0; i<=7; i++){
        if (0b10000000 & r){
            write_1();
        }
        else {
            write_0();
        }      
        r = r<<1;
    }
    
    //green
    int j=0;
    for (j=0; j<=7; j++){
        if (0b10000000 & g){
            write_1();
        }
        else {
            write_0();
        }    
        
        g = g<<1;
    }
    
    //blue
    int k=0;
    for (k=0; k<=7; k++){
        if (0b10000000 & b){
            write_1();
        }
        else {
            write_0();
        }    
        
        b = b<<1;
    }
    
    LATA = 0;
    delay50us();
}
 
uint32_t Wheel(unsigned char WheelPos) {
 WheelPos = 255 - WheelPos;
 if(WheelPos < 85) {
 return packColor(255 - WheelPos * 3, 0, WheelPos * 3);
 }
 if(WheelPos < 170) {
 WheelPos -= 85;
 return packColor(0, WheelPos * 3, 255 - WheelPos * 3);
 }
 WheelPos -= 170;
 return packColor(WheelPos * 3, 255 - WheelPos * 3, 0);
}

int main(void) {
    setup();
    unsigned char a=0;
    while(1) {
        LATA = 0;
        //delay50us();
       
        delay(FRAME_PER);
        
       //int frame=0;
       
        
        
        writePacCol(Wheel(a++));
        
        /*while(frame<=255){
            drawFrame(frame);
            delay(FRAME_PER);
            frame++;     
        }
        while (frame>=0){
            drawFrame(frame);
            delay(FRAME_PER);
            frame--;
        }*/
        
    }
    return 0;
}


