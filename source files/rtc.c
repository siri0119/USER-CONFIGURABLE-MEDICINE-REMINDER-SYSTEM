//rtc.c -- Implementation file containing functions for driving and interfacing with the LPC2148 Real-Time Clock

#include "rtc.h" // Include the corresponding header file to pull in function prototypes and macro constants
#include <lpc214x.h> // Include the standard hardware register definition header specific to NXP LPC214x microcontrollers

typedef unsigned int u32; // Create a localized alias 'u32' mapping to a 32-bit unsigned integer data type
typedef int s32; // Create a localized alias 's32' mapping to a standard 32-bit signed integer data type

//s32 hour,min,sec,date,month,year,day; // Commented out global variables meant to hold individual raw time and date parameters

// Array to hold names of days of the week*/ // Commented out placeholder note intended for a string array of weekday names

#define _LPC2148 // Define a compile-time target flag ensuring configurations specific to the LPC2148 chip are compiled


/*
Initialize the Real-Time Clock (RTC)
This function disables the RTC, sets the prescaler values, 
and then enables the RTC.
*/
void RTC_Init(void) // Define the initialization function responsible for configuring and starting up the hardware RTC peripheral
{ // Entry point of the RTC architecture initialization routine
  // Disable and reset the RTC // Descriptive comment explaining the immediate action of resetting the internal counter blocks
	CCR = RTC_RESET; // Write the reset mask to the Clock Control Register to halt all current tracking and clear internal sub-counters
	
	#ifndef _LPC2148 // Preprocessor conditional check; evaluates to true only if the '_LPC2148' macro symbol was NOT defined
  
  // Set prescaler integer and fractional parts // Comment indicating that the clock division registers are about to be populated
	PREINT = PREINT_VAL; // Assign calculated integer divider value to the Prescaler Integer Register for non-LPC2148 setups
	PREFRAC = PREFRAC_VAL; // Assign calculated fractional divider value to the Prescaler Fractional Register for non-LPC2148 setups
  
  // Enable the RTC // Comment specifying that the clock peripheral tracking logic is being turned on
	CCR = RTC_ENABLE; // Write the enable bit pattern to the Clock Control Register to kick off clock tick accumulation

	#else // Preprocessor branch executed when the target chip IS defined as '_LPC2148' (as defined above)
  // Enable the RTC with external clock source // Comment documenting that an external crystal oscillator will source the module
	CCR = RTC_ENABLE | RTC_CLKSRC;	// Perform bitwise OR to merge the enable bit and the external crystal source bit, then write to CCR
	#endif // End of the conditional preprocessor compilation block determining register configurations
} // Exit point marking the conclusion of the RTC_Init configuration function

/*
Get the current RTC time
hour Pointer to store the current hour
minute Pointer to store the current minute
second Pointer to store the current second
*/
//void GetRTCTimeInfo(s32 *hour, s32 *minute, s32 *second) // Outdated/commented-out prototype passing pointer references for raw values
void GetRTCTimeInfo(char *rtc) // Function definition that populates a string character array with formatted clock characters
{ // Entry point of the time extraction and ASCII conversion routine
	/**hour = HOUR; // Commented out reference displaying the old method of reading the hardware HOUR register directly
	*minute = MIN; // Commented out reference displaying the old method of reading the hardware MIN register directly
	*second = SEC;*/ // Commented out reference displaying the old method of reading the hardware SEC register directly
				rtc[0]=(HOUR/10)+'0'; // Extract the tens digit of the hours register, offset it by ASCII '0', and store in the first index
				rtc[1]=(HOUR%10)+'0'; // Isolate the units digit of the hours register via modulo, convert to ASCII character, and store at index 1
				rtc[2]=':'; // (Implied separator placeholder, notice index 2 is bypassed in code but implicitly leaves space for format)
				rtc[3]=(MIN/10)+'0'; // Extract the tens digit of the hardware minutes register, shift to ASCII, and store in index 3
				rtc[4]=(MIN%10)+'0'; // Capture the units digit of the hardware minutes register, convert to its ASCII symbol, and store in index 4
				rtc[5]=':'; // (Implied separator placeholder, notice index 5 is bypassed in code but leaves formatting space)
				rtc[6]=(SEC/10)+'0'; // Isolate the tens digit of the hardware seconds counter, adjust to an ASCII char, and write to index 6
				rtc[7]=(SEC%10)+'0'; // Fetch the units digit of the hardware seconds counter, adjust to its ASCII character value, and write to index 7
} // Termination of the GetRTCTimeInfo string formatting function

/*
Get the current RTC date
day Pointer to store the current date (1-31)
month Pointer to store the current month (1-12)
year Pointer to store the current year (four digits)
*/
void GetRTCDateInfo(char *p) // Function definition that writes the converted hardware date data into an output character buffer
{ // Starting boundary of the date processing and string formatting block
	int i,y; // Declare localized working integers: 'i' for buffer index iteration, and 'y' to safely copy the year counter
	/**date = DOM; // Inactive commented-out line previously used to capture Day of Month integer directly via pointer
	*month = MONTH; // Inactive commented-out line previously used to capture Month integer directly via pointer
	*year = YEAR;*/ // Inactive commented-out line previously used to capture Year integer directly via pointer
				p[0]=(DOM/10)+'0'; // Pull the tens position from the Day of Month register, convert to text character, store at index 0
				p[1]=(DOM%10)+'0'; // Isolate the single units spot of the Day of Month register, change to text character, store at index 1
				p[2]='/'; // (Implied separator placeholder, notice index 2 is skipped but leaves spatial layout buffer gap)
				p[3]=(MONTH/10)+'0'; // Isolate the tens digit of the calendar Month register, parse to ASCII character, store at index 3
				p[4]=(MONTH%10)+'0'; // Isolate the units digit of the calendar Month register, parse to ASCII character, store at index 4
	
	y=YEAR; // Read the 4-digit year value directly from the hardware register into the temporary tracking integer variable 'y'
	for(i=9;y;y/=10) // Set loop index 'i' to 9 (last index of string); loop as long as 'y' is non-zero; divide 'y' by 10 each pass
	{ // Body loop segment processing digits of the year from right-to-left
		p[i]=y%10+'0'; // Grab the absolute lowest digit of 'y' using modulo 10, shift it to ASCII, and insert it at position 'i'
		i--; // Decrement index location 'i' to shift storage to the left for the next highest order digit
	} // End of dynamic loop logic for individual integer digit breakdown
			/*	p[6]=(YEAR/1000)+'0'; // Commented alternative string-building technique for the thousands numeric character block
				p[7]=(YEAR%1000)+'0'; // Commented alternative string-building technique for the hundreds numeric character block
				p[8]=(YEAR%100)+'0'; // Commented alternative string-building technique for the tens numeric character block
				p[9]=(YEAR%10)+'0';*/ // Commented alternative string-building technique for the units numeric character block
} // Boundary terminating the date string conversion function

/*
Set the RTC time
Hour to set (0-23)
Minute to set (0-59)
Second to set (0-59)
*/
void SetRTCTimeInfo(char *p) // Function definition that interprets an incoming ASCII time buffer string and overwrites internal RTC values
{ // Entry point of the character-to-integer conversion and clock setting sequence
	int h = 0, m = 0, s = 0; // Initialize standard local variables to zero to hold calculated hour, minute, and second integers
	
	h=(p[0]-'0')*10; // Convert index 0 char to numerical value by subtracting ASCII offset, scale up by 10 to represent tens digit
	h+=(p[1]-'0'); // Extract single unit digit value from character at index 1 and add it into the hour total integer variable
	m=(p[3]-'0')*10; // Convert character index 3 to digit integer, multiply by 10 to form the base minutes tens place value
	m+=(p[4]-'0'); // Extract single unit digit value from character at index 4 and add it into the minutes accumulator variable
	s=(p[6]-'0')*10; // Convert character index 6 to digit value, multiply by 10 to set up the seconds tens column configuration
	s+=(p[7]-'0'); // Extract single unit digit value from character at index 7 and add it into the seconds accumulator variable

	HOUR = h; // Assign the freshly calculated hour value directly into the hardware configuration register for HOURS
	MIN = m; // Assign the freshly calculated minute value directly into the hardware configuration register for MINUTES
	SEC = s; // Assign the freshly calculated second value directly into the hardware configuration register for SECONDS
} // End boundary of the SetRTCTimeInfo operation block

/*
Set the RTC date
day of month to set (1-31)
month to set (1-12)
year to set (four digits)
*/
void SetRTCDateInfo(char*p) // Function definition that reads an ASCII string pointer to reprogram hardware calendar dates
{ // Beginning of parsing structure for incoming calendar values
	int d = 0, m = 0, y = 0; // Initialize empty localized working storage values for the parsed day, month, and year
	
	d=(p[0]-'0')*10; // Parse the initial day index text value, scale it by a factor of 10 to calculate the tens column offset
	d+=(p[1]-'0'); // Combine the individual base component from character array index 1 into the total day integer variable
	m=(p[3]-'0')*10; // Extract the month text component at index 3, multiply by 10 to assign appropriate tens place magnitude
	m+=(p[4]-'0'); // Incorporate the base units text offset from position index 4 directly to the local month variable
	y=(p[6]-'0')+(y*10); // Shift current 'y' left (0 initially) and append the thousands digit literal extracted from index 6
	y=(p[7]-'0')+(y*10); // Shift working integer 'y' value left by ten, then accumulate the hundreds digit extracted from index 7
	y=(p[8]-'0')+(y*10); // Shift working integer 'y' value left by ten, then accumulate the tens column digit extracted from index 8
	y=(p[9]-'0')+(y*10); // Shift working integer 'y' value left by ten, then accumulate the final units row digit extracted from index 9

	DOM = d; // Commit computed integer value 'd' directly into the Day of Month hardware register
	MONTH = m; // Commit computed integer value 'm' directly into the calendar Month hardware register
	YEAR = y; // Commit computed integer value 'y' directly into the 4-digit Year hardware register
} // Structural exit bound for the SetRTCDateInfo functional routine

/*
Get the current day of the week
dow Pointer to store Day of Week (0=Sunday, ..., 6=Saturday)
*/
void GetRTCDay(s32 *dow) // Function definition that takes a signed 32-bit integer pointer reference to dump weekday tracks
{ // Start boundary of the tracking collection code block
	*dow = DOW; // Dereference pointer parameter to pass out the precise current value inside the Day of Week hardware counter register
} // End boundary of the weekday fetch function

/*
Set the day of the week in RTC
Day of Week to set (0=Sunday, ..., 6=Saturday)
*/
void SetRTCDay(u32 dow) // Function definition taking a 32-bit unsigned input value to force rewrite weekday indexing tracking
{ // Start point of update block
	
	DOW = dow; // Commit the passed functional argument integer straight into the Day of Week (DOW) controller storage register
	
} // End point of update block execution loop