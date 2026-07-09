// RTC header file

//#include <lpc214x.h> 
// Include LCD header files
#include "lcd.h" // Include LCD functions
#include "lcd_defines.h" // Include LCD definitions

// Clock values
#define FOSC 12000000 // Oscillator frequency
#define CCLK (5*FOSC) // CPU clock
#define PCLK (CCLK/4) // Peripheral clock

// RTC prescaler values
// RTC clock value
// Prescaler registers

// PREINT calculation
// PREFRAC calculation
// LPC2129 manual reference

#define PREINT_VAL (int) ((PCLK / 32768) - 1) // Prescaler integer value
#define PREFRAC_VAL (PCLK -((PREINT_VAL + 1) * 32768)) // Prescaler fractional value

// RTC control register bits
// Clock enable bit
#define RTC_ENABLE (1<<0) // Enable RTC

// Clock reset bit
#define RTC_RESET (1<<1) // Reset RTC
 

// LPC2148 clock source
// Clock source select bit
// External clock source
// Internal clock source
#define RTC_CLKSRC (1<<4) // RTC clock source


// Day values
#define SUN 0 // Sunday
#define MON 1 // Monday
#define TUE 2 // Tuesday
#define WED 3 // Wednesday
#define THU 4 // Thursday
#define FRI 5 // Friday
#define SAT 6 // Saturday

// Unsigned integer type
typedef unsigned int u32;

// Signed integer type
typedef int s32;


// Initialize RTC
void RTC_Init(void);

// Get RTC time
void GetRTCTimeInfo(char*);

// Get RTC date
void GetRTCDateInfo(char*);


// Set RTC time
void SetRTCTimeInfo(char*);

// Set RTC date
void SetRTCDateInfo(char*);

// Get RTC day
void GetRTCDay(s32 *dow);

// Set RTC day
void SetRTCDay(u32);
