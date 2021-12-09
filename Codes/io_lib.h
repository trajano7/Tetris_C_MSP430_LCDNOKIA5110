#ifndef IO_LIB_H_
#define IO_LIB_H_

#include <msp430.h>

#define TRUE 1
#define FALSE 0

volatile char newCommand;              //Flag que indica se tem um novo input do joystick
volatile char flagUpdateLCD;           //Flag que indica se vai ter atualizacao do LCD

void resetAnimation();

void USCI_B1_config();
void sendData(char data);
void sendCommand(char command);
void lcd_init();
void setCoordinate(char x, char y);
void clearLCD();
void lcdUpdate();

void ad_config();

void printChar(char number);
void printDec(int number);

void printPoints();
void printHighScore();

void delay_us(unsigned int us);
void delay_ms(unsigned int ms);


#endif /* IO_LIB_H_ */
