//rtc.h -- Interfacing header defining architecture configurations, scaling macros, and functional prototypes for the LPC2148 RTC

//#include <lpc214x.h> // Commented out hardware register inclusion macro reference line

//include LCD header files // Code module notation alerting dependencies required for linked visualization screens
#include "lcd.h" // Pull in the core liquid crystal display peripheral configuration structures and controls
#include "lcd_defines.h" // Link in mapped alphanumeric symbolic registers, macros, pin names, and LCD layouts

// System clock and peripheral clock Macros // Grouping comments establishing crystal operating frequencies
#define FOSC 12000000 // Define baseline fundamental external crystal oscillator speed at exactly 12 MHz
#define CCLK (5*FOSC) // Multiply base oscillator clock by 5 using Phase-Locked Loop to generate a 60 MHz core processing clock (CCLK)    
#define PCLK (CCLK/4) // Divide system core clock speed by 4 to define the active Peripheral Clock (PCLK) pipeline rate of 15 MHz

// RTC Prescaler Calculation Macros // Section description notes outlining formulas used for clock synthesis
// RTC requires 32.768 kHz clock for 1-second increment. // Clarifying requirement statement for strict internal tracking timebases
// PREINT and PREFRAC registers divide PCLK to generate 32.768 kHz. // Explicitly naming tracking divider networks in the chip core

// PREINT = int (PCLK / 32768) - 1; // Comment tracking algorithmic formula layout for integer configuration register matching
// PREFRAC = PCLK -((PREINT + 1) * 32768); // Comment tracking architectural formula layout for resolving remaining fraction values
// Note: This information collected from LPC2129 Manual // Document source origin acknowledgement annotation referencing architecture spec sheets

#define PREINT_VAL (int) ((PCLK / 32768) - 1) // Macro deriving the whole integer division constant required for the prescaler register setup
#define PREFRAC_VAL (PCLK -((PREINT_VAL + 1) * 32768)) // Macro deriving fractional remainder clocks to minimize errors in periodic ticks

//RTC Control Register (CCR) Bit Definitions // Context layout describing active operational status control bit assignments
// Bit 0 – Clock Enable --> 1 = Enable RTC counters  0 = Disable RTC counters // Bit annotation explaining control logic for setting counter enable bits
#define RTC_ENABLE (1<<0) // Define shifting mask enabling the clock module by setting bit 0 to high logic high state

// Bit 1 – Clock Reset --> 1 = Reset RTC counters    0 = Normal operation // Bit annotation clarifying register behavior during reset states
#define RTC_RESET (1<<1) // Establish functional mask targeting hardware register reset by forcing bit 1 position high
 

//only for LPC2148 // Special hardware context warning comment notifying deviations unique to targeted processor models
// Bit 4 – Clock Source Select // Identification annotation highlighting functional shifts mapping to register bit 4
// 1 = Use external 32.768 kHz oscillator // Detailed definition mapping behavior of high logic inputs on hardware crystal path selectors
// 0 = Use internal PCLK as RTC clock source // Detailed definition detailing structural internal peripheral fallback configurations
#define RTC_CLKSRC (1<<4) // Define mask bit 4 to actively map peripheral tracking input structures specifically onto external crystal pins


#define SUN 0 // Assign numeric identifier value 0 to signify the calendar day tracking point for Sunday
#define MON 1 // Assign numeric identifier value 1 to signify the calendar day tracking point for Monday
#define TUE 2 // Assign numeric identifier value 2 to signify the calendar day tracking point for Tuesday
#define WED 3 // Assign numeric identifier value 3 to signify the calendar day tracking point for Wednesday
#define THU 4 // Assign numeric identifier value 4 to signify the calendar day tracking point for Thursday
#define FRI 5 // Assign numeric identifier value 5 to signify the calendar day tracking point for Friday
#define SAT 6 // Assign numeric identifier value 6 to signify the calendar day tracking point for Saturday
typedef unsigned int u32; // Standard internal global export type-aliasing to secure 32-bit unsigned structures
typedef int s32; // Standard internal global export type-aliasing to secure signed integer numeric ranges


void RTC_Init(void); // Function prototype declaring external visibility for resetting and initializing clock structures
void GetRTCTimeInfo(char*); // Global reference prototype detailing access constraints for reading text time representations
void GetRTCDateInfo(char*); // Global reference prototype detailing access constraints for translating dates to character sets


void SetRTCTimeInfo(char*); // Global functional prototype providing string input parameters to reprogram ticking clocks
void SetRTCDateInfo(char*); // Global functional prototype providing string input parameters to overwrite hardware dates

void GetRTCDay(s32 *dow); // Prototype defining call interfaces returning tracking weekday representations via pointers
void SetRTCDay(u32); // Prototype defining structure paths designed to override and store weekday index information