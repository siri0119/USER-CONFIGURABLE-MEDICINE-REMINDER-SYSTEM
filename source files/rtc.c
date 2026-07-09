// RTC functions

#include "rtc.h" 
#include <lpc214x.h> 

typedef unsigned int u32; 
typedef int s32; 

//s32 hour,min,sec,date,month,year,day;

// Day names array

#define _LPC2148 // Define LPC2148


// Initialize RTC
void RTC_Init(void)
{
	// Reset RTC
	CCR = RTC_RESET;
	
	#ifndef _LPC2148
  
	// Set prescaler values
	PREINT = PREINT_VAL;
	PREFRAC = PREFRAC_VAL;
  
	// Enable RTC
	CCR = RTC_ENABLE;

	#else

	// Enable RTC with clock source
	CCR = RTC_ENABLE | RTC_CLKSRC;

	#endif
}


// Get current RTC time
//void GetRTCTimeInfo(s32 *hour, s32 *minute, s32 *second)
void GetRTCTimeInfo(char *rtc)
{
	/**hour = HOUR;
	*minute = MIN;
	*second = SEC;*/

	// Store hour tens digit
	rtc[0]=(HOUR/10)+'0';

	// Store hour units digit
	rtc[1]=(HOUR%10)+'0';

	// Store time separator
	rtc[2]=':';

	// Store minute tens digit
	rtc[3]=(MIN/10)+'0';

	// Store minute units digit
	rtc[4]=(MIN%10)+'0';

	// Store time separator
	rtc[5]=':';

	// Store second tens digit
	rtc[6]=(SEC/10)+'0';

	// Store second units digit
	rtc[7]=(SEC%10)+'0';
}


// Get current RTC date
void GetRTCDateInfo(char *p)
{
	// Loop and year variables
	int i,y;

	/**date = DOM;
	*month = MONTH;
	*year = YEAR;*/

	// Store date tens digit
	p[0]=(DOM/10)+'0';

	// Store date units digit
	p[1]=(DOM%10)+'0';

	// Store date separator
	p[2]='/';

	// Store month tens digit
	p[3]=(MONTH/10)+'0';

	// Store month units digit
	p[4]=(MONTH%10)+'0';
	
	// Store year value
	y=YEAR;

	// Store year digits
	for(i=9;y;y/=10)
	{
		// Store year digit
		p[i]=y%10+'0';

		// Move to previous position
		i--;
	}

	/*	p[6]=(YEAR/1000)+'0';
		p[7]=(YEAR%1000)+'0';
		p[8]=(YEAR%100)+'0';
		p[9]=(YEAR%10)+'0';*/
}


// Set RTC time
void SetRTCTimeInfo(char *p)
{
	// Hour, minute and second variables
	int h = 0, m = 0, s = 0;
	
	// Get hour tens digit
	h=(p[0]-'0')*10;

	// Add hour units digit
	h+=(p[1]-'0');

	// Get minute tens digit
	m=(p[3]-'0')*10;

	// Add minute units digit
	m+=(p[4]-'0');

	// Get second tens digit
	s=(p[6]-'0')*10;

	// Add second units digit
	s+=(p[7]-'0');

	// Set RTC hour
	HOUR = h;

	// Set RTC minute
	MIN = m;

	// Set RTC second
	SEC = s;
}


// Set RTC date
void SetRTCDateInfo(char*p)
{
	// Date, month and year variables
	int d = 0, m = 0, y = 0;
	
	// Get date tens digit
	d=(p[0]-'0')*10;

	// Add date units digit
	d+=(p[1]-'0');

	// Get month tens digit
	m=(p[3]-'0')*10;

	// Add month units digit
	m+=(p[4]-'0');

	// Get first year digit
	y=(p[6]-'0')+(y*10);

	// Get second year digit
	y=(p[7]-'0')+(y*10);

	// Get third year digit
	y=(p[8]-'0')+(y*10);

	// Get fourth year digit
	y=(p[9]-'0')+(y*10);

	// Set RTC date
	DOM = d;

	// Set RTC month
	MONTH = m;

	// Set RTC year
	YEAR = y;
}


// Get current RTC day
void GetRTCDay(s32 *dow)
{
	// Store RTC day
	*dow = DOW;
}


// Set RTC day
void SetRTCDay(u32 dow)
{
	// Set RTC day
	DOW = dow;
	
}
