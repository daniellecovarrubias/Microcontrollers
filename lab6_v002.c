#include <p24Fxxxx.h>
#include <xc.h>
#include <assert.h>
#include "stdint.h"
#include "covar014_lab2b_asmlib_v001.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// PIC24FJ64GA002 Configuration Bit Settings
// 'C' source line config statements

// CONFIG2
#pragma config POSCMOD = NONE           // Primary Oscillator Select (Primary oscillator disabled)
#pragma config I2C1SEL = PRI            // I2C1 Pin Location Select (Use default SCL1/SDA1 pins)
#pragma config IOL1WAY = OFF            // IOLOCK Protection (IOLOCK may be changed via unlocking seq)
#pragma config OSCIOFNC = OFF           // Primary Oscillator Output Function (OSC2/CLKO/RC15 functions as CLKO (FOSC/2))
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

// A circular buffer implementation: it has a "read" pointer, a "write" pointer,
// and a "numElemsInBuff" value.
// Compare this code to the one in buffer1.c.
//
// It makes more sense to use the UART unit in showing how the buffer works,
// but since mplabx doesn't fully simulate UART, I use an artificial setup:
// int0 is raised when the source device has data ready on RB0-RB7.
// The "processing" of data is to keep the sum of 4*x. I intentionally
// chose such a ridiculous computation so that it uses tens of instruction cycles,
// giving us enough time to emulate different scenarios such as the source
// sending bursts of data followed by silent periods.

unsigned int calculateAVG(unsigned int data[]);
unsigned int calculateSD(unsigned int data[]);
void setup(void);
void lcd_init();
void lcd_cmd(char command);
void lcd_printChar(char command);
void lcd_clear();
void lcd_setCursor(int x, int y);
void lcd_printStr(const char *s);



#define BUFFER_SIZE 1024

volatile unsigned int buffer[BUFFER_SIZE];
volatile unsigned int standard_deviation = 0;
volatile unsigned int average = 0;
volatile unsigned int sampledata[32];
volatile unsigned int index = 0;





void __attribute__((__interrupt__)) _ADC1Interrupt(void) {
    
    _AD1IF = 0;
    
    buffer[index] = ADC1BUF0;
    
    if(index==BUFFER_SIZE){
        index=0;
    }
    
    else{
        index++;
    }
}

void __attribute__((__interrupt__,__auto_psv__)) _T2Interrupt(void) {
    
    _T2IF = 0;
    
    int i;
    int idx= BUFFER_SIZE;
    
    
    for (i=0 ; i<32 ; i++) { 
            
        if (idx < 0){
            idx = BUFFER_SIZE;
        }
        
        sampledata[i]= buffer[idx];
        
        idx--;
    }
}


unsigned int calculateAVG(unsigned int data[])
{
    unsigned int sum = 0, mean;

    int i;

    for(i=0; i<32; i++)
    {
        sum += data[i];
    }

    mean = sum/32;
    return mean;
}

unsigned int calculateSD(unsigned int data[])
{
    unsigned int sum = 0, mean, standardDeviation = 0;

    int i;

    for(i=0; i<32; i++)
    {
        sum += data[i];
    }

    mean = sum/32;

    for(i=0; i<10; ++i)
        standardDeviation += pow(data[i] - mean, 2);

    return sqrt(standardDeviation/32);
}

void setup(void)
{
    CLKDIVbits.RCDIV = 0; // make 16MHz
    AD1PCFG = 0xfffe; // AN0 is the only analog pin we are using.
    TRISA=0;
    TRISAbits.TRISA0=1;
    
    
    T1CON = 0; // turn off timer, prescalar 1:1, use Tcy as clk source
    PR1 = 25000;
    TMR1 = 0;
    _T1IF = 0;
    T3CONbits.TCKPS = 2; //prescalar 64
    T1CONbits.TON = 1; // now turn on the timer
    
    // setup Timer 3
    T3CON = 0;
    TMR3 = 0;
    PR3 = 15625;
    T3CONbits.TCKPS = 2; // pre 1:64
    // we do not care about T3IF here. Don't reset it,
    // don't check it.
    
    //CNPU1bits.CN2PUE = 1;//RA0
    I2C2CONbits.I2CEN = 0;
    I2C2BRG = 157;
    _MI2C2IF = 0;
    I2C2CONbits.I2CEN = 1;
    
    // Setup on A/D
    AD1CON1 = 0;
    AD1CON2 = 0;
    AD1CON3 = 0;
    AD1CHS  = 0;
    
     AD1CON1bits.ASAM = 1; // auto-sampling. P21, Sec. 17.7.3.3 in the ADC
       // document says use ASAM=1
    AD1CON1bits.SSRC = 0b010; // Use Timer3 as the conversion trigger
       // We can also use Timer3 / Timer2 as a 32-bit timer to trigger
       // A/D sampling/conversion, because the 32-bit timer
       // will also generate the T3IF event.

    //doesn't matter //AD1CON3bits.SAMC = 3; // auto-sample time = X * TAD (what is X?)
    AD1CON3bits.ADCS = 1; // clock = Y * Tcy (what is Y?)
    

    // Configure A/D interrupt
    _AD1IF = 0; // clear flag
    _AD1IE = 1; // enable interrupt
    _AD1IP = 5;

    AD1CON1bits.ADON = 1;

    
    
    T2CON = 0;
    PR2 = 15625;
    T2CONbits.TCKPS = 2;
    _T2IP = 4;  // set it as low priority interrupt
    _T2IE = 1;
    
    // turn on Timer 2&3
    T2CONbits.TON = 1;
    T3CONbits.TON = 1;
    

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

void lcd_setCursor(int x, int y){
    //row y
    //column x
    char com = (char)((0x40*y + x) + 0b10000000);
    lcd_cmd(com);
}

void lcd_printStr(const char *s){
    int i = 0;
    for(i=0; i < strlen(s) ; i++){
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



int main(void)
{
   
    setup();
    lcd_init();
    char sd[20];
    char avg[20];

    while (1) {
        while (!_T1IF);
        _T1IF = 0;
       
        
        
        sprintf(avg, "%6.4f V", (3.3/1024)*calculateSD(sampledata));
        sprintf(sd, "%3.1f mV", (3.3/1024)*calculateAVG(sampledata));
        
        lcd_clear();
        delay1ms();
        delay1ms();
        lcd_setCursor(0,0);
        lcd_printStr(avg);
        lcd_setCursor(0,1);
        lcd_printStr(sd);
    }
}


