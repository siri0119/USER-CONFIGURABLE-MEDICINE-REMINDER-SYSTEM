// Include LPC21xx header file
#include<lpc21xx.h>

// Include RTC header file
#include "rtc.h"


// EINT0 input pin
#define EINT0_INPUT_PIN 0x0000000C

// EINT0 VIC channel number
#define EINT0_VIC_CHN0 14


// EINT1 input pin
#define EINT1_INPUT_PIN 0x000000C0

// EINT1 VIC channel number
#define EINT1_VIC_CHN0 15


// Buzzer pin
#define BUZZER 23


// Initialize interrupts
void Init_intrrupt(void);

// EINT0 interrupt function
void eint0_isr(void)__irq;

// EINT1 interrupt function
void eint1_isr(void)__irq;

// Compare two values
int check(char*p,char*q);

// Open RTC menu
void menu2(void);

// Open configuration menu
void configure(void);


// Unsigned integer type
typedef unsigned int u32;
