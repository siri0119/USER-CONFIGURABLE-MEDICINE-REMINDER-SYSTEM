// ==================================================================================
// FILE: interrupt.h
// DESCRIPTION: Header configuration for VIC routing, vector maps, and UI prototypes
// ==================================================================================

#include<lpc21xx.h>           // Include peripheral register map structures for the NXP LPC21xx series microcontroller ARM7TDMI-S core.

#include "rtc.h"              // Pull in underlying dependencies and sub-system declarations related to the hardware Real-Time Clock module.

//EINT0                       // Label group designated to consolidate hardware parameters for External Interrupt 0 line routing.
#define EINT0_INPUT_PIN 0x0000000C // Define PINSEL0 bitmask value to switch Pin 0.1 alternate functionality over to EINT0 operation mode.
#define EINT0_VIC_CHN0 14     // Assign index channel constant representing the fixed core hardware allocation mapping for EINT0 inside the VIC.
//EINT1                       // Label group designated to consolidate hardware parameters for External Interrupt 1 line routing.
#define EINT1_INPUT_PIN 0x000000C0 // Define PINSEL0 bitmask value to switch Pin 0.3 alternate functionality over to EINT1 operation mode.
#define EINT1_VIC_CHN0 15     // Assign index channel constant representing the fixed core hardware allocation mapping for EINT1 inside the VIC.

#define BUZZER 23             // Map an explicit bit offset identifier pairing the alert sound buzzer transducer to GPIO Pin P0.23.

void Init_intrrupt(void);     // Declare the initialization routine responsible for configuring pin-selects, edge-triggers, and VIC slots.
void eint0_isr(void)__irq;    // Prototype the low-overhead hardware ISR handler for External Interrupt 0 using the special __irq compiler keyword attribute.
void eint1_isr(void)__irq;    // Prototype the low-overhead hardware ISR handler for External Interrupt 1 using the special __irq compiler keyword attribute.
int check(char*p,char*q);     // Profile a baseline string checking function utilized to match current RTC times against target medical reminders.
void menu2(void);             // Prototype sub-layer display selection loop designated to handle individual real-time component updates.
void configure(void);         // Prototype the master interactive configuration terminal environment coordinating nested setup sub-menus.


typedef unsigned int u32;     // Create a compact shorthand type alias naming unsigned 32-bit integer structures as 'u32'.