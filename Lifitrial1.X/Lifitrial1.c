/*
 * File:   i2c_master.c
 * Author: Abhi
 *
 * Created on April 21, 2015, 8:00 PM
 */
//MASTER
//---------------------------------------------------//

#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include<pic16f886.h>
// CONFIG1
#pragma config "FOSC = INTRC_NOCLKOUT"          // Oscillator Selection bits (INTRC_NOCLKOUT oscillator: Internal Oscillator with No Clock Output on Pins)
#pragma config "WDTE = OFF"                     // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config "PWRTE = OFF"                    // Power-up Timer Enable bit (PWRT disabled)
#pragma config "MCLRE = ON"                     // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config "CP = OFF"                       // Code Protection bit (Program memory code protection is disabled)
#pragma config "CPD = OFF"                      // Data Code Protection bit (Data memory code protection is disabled)
#pragma config "BOREN = ON"                     // Brown Out Reset Selection bits (BOR disabled)
#pragma config "IESO = OFF"                     // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config "FCMEN = OFF"                    // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config "LVP = OFF"                      // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config "BOR4V = BOR40V"                 // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config "WRT = OFF"                      // Flash Program Memory Self Write Enable bits (Write protection off)


#define _XTAL_FREQ 8000000
#define baud_rate 9600
#define i2c_bus_rate 400000L

char UART_Init(long);
void UART_Write(unsigned char);
char UART_TX_Empty(void);
void UART_Write_Text(const char *);
void i2c_idle(void);
char UART_Data_Ready(void);
char UART_Read(void);
unsigned char * UART_Read_Text(void);
unsigned char getch(void);
unsigned char getche(void);

//Delay functions
void delay_us(unsigned int);
void delay_ms(unsigned int);
void delay1ms(void);


//I2C functions
void I2C_init(void);
void I2C_Start(void);
void I2C_Stop(void);
void I2C_Write(unsigned char);
void I2C_Write_Text(const char *);
char I2C_address_send();
char I2C_address_send1();
char I2C_address_send2();
//I2C Constants
#define ACK	1		//ACK required for page R/W
#define NO_ACK	0		//NO ACK for byte R/W
#define MEMADD_8_16 1	//MEM Address 8bit-0/16bit-1
#define EOS 0x00		//End of String NULL

//bit IC2_ack;
//bit ACK_Bit;
bit IC2_ack;
  unsigned char *is;
//Serial EEPROM pins
//#define SDA PORTCbits.PORTC4;		// connect to SDA pin (Data) of 24Cxx
//#define SCL PORTCbits.PORTC3;		// connect to SCL pin (Clock) of 24Cxx

void main(void)
{
   //const unsigned char *msg;

   const unsigned char * arr1 = "\r\ntaking in the text \r\n";

   const unsigned char *arr2="\r\nAcknowledged\r\n";

   const unsigned char *arr3= "You have entered:\r\n";

   const unsigned char *arr4= "UART Initialised\r\n";

   const unsigned char *arr5= "Sending TO Led\r\n";

   const unsigned char *arr6= "Tx of 1 byte complete\r\n";

   const unsigned char *arr7= "Tx completed all bytes\r\n";

   unsigned int j;

   unsigned char LED_Output;

  TRISB=0x00;


   OSCCONbits.IRCF = 0x07;  // Configure Internal OSC for 8MHz Clock

    while(!OSCCONbits.HTS);   // Wait Until Internal Osc is Stable

    INTCON=0;   // purpose of disabling the interrupts.

    UART_Init(baud_rate);

    UART_Write_Text(arr4);

    delay_ms(500);

    while(1)
    {


        UART_Write_Text(arr1);

         is=UART_Read_Text();
         
         UART_Write_Text(arr2);
         
         UART_Write_Text(arr3);
         
         UART_Write_Text(is);
         //sending to led
         UART_Write_Text(arr5);

       while(*is)
         {
              for(j=0;j<=7;j++)
              {
                LED_Output= (*is&0x01)==1?1:0;
                RB0=LED_Output;
                delay_ms(125);
                *is=*is>>1;
             }
              is++;
            UART_Write_Text(arr6);
        }
         UART_Write_Text(arr7);
       
     }

         
     
         


 }





//Delay routines

void delay_us(unsigned int i)
{
	for (;i!=0x00;i--);
}

void delay_ms(unsigned int i)
{
	for(;i!=0x00;i--)
		delay1ms();
}

void delay1ms(void)
{
	unsigned int j = 130;
	for (;j!=0x00;j--);
}

  char UART_Init(long baudrate)
{
  unsigned int x;
  x = (_XTAL_FREQ - baudrate*64)/(baudrate*64); //SPBRG for Low Baud Rate
  if(x>255) //If High Baud Rate required
  {
    x = (_XTAL_FREQ - baudrate*16)/(baudrate*16); //SPBRG for High Baud Rate

    BRGH = 1; //Setting High Baud Rate
    SPBRG = x; //Writing SPBRG register
    SYNC = 0; //Selecting Asynchronous Mode
    SPEN = 1; //Enables Serial Port
    CREN = 1; //Enables Continuous Reception
    TXEN = 1; //Enables Transmission
  }
  if(x<256)
  {
    BRGH = 0; //Setting low Baud Rate
    SPBRG = x; //Writing SPBRG register
    SYNC = 0; //Selecting Asynchronous Mode
    SPEN = 1; //Enables Serial Port
    CREN = 1; //Enables Continuous Reception
    TXEN = 1; //Enables Transmission
    return 1;
  }
  return 0;
}

void UART_Write(unsigned char data)
{
  while(!PIR1bits.TXIF);
  while(!TRMT); //Waiting for Previous Data to Transmit completly
  TXREG = data; //Writing data to Transmit Register, Starts transmission
}

char UART_TX_Empty()
{
  return TRMT; //Returns Transmit Shift Status bit
}

void UART_Write_Text(const char *text)
{
  int i;
  for(i=0;text[i]!='\0';i++)
    UART_Write(text[i]);

}

//recive part
char UART_Data_Ready()
{
  return RCIF;
}

char UART_Read()
{
  while(!RCIF); //Waits for Reception to complete
  return RCREG; //Returns the 8 bit data
}

unsigned char * UART_Read_Text()
{
  unsigned const char *a="Keyed in \r\n";

  unsigned static char string[20];

  unsigned char x, i = 0;

while((x = UART_Read()) != 13)
{
    
 //and store the received characters into the array string[] one-by-one
string[i++] = x;
}

//insert NULL to terminate the string
string[i] = '\0';
  UART_Write_Text(a);

//return the received string
return(string);
}

void putch(unsigned char byte)
{
    /* output one byte */
    while(!TXIF)    /* set when register is empty */
        continue;
    TXREG = byte;
}

unsigned char getch()
{
    /* retrieve one byte */
    while(!RCIF)    /* set when register is not empty */
        continue;
    return RCREG;
}

unsigned char getche(void)
{
    unsigned char c;
    putch(c = getch());
    return c;
}

void i2c_idle(void)
{
   while((SSPSTATbits.R));

   while((!((SSPCON2 & 0x1F) == 0x00)));
}

