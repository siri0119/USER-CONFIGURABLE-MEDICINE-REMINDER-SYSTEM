#include<lpc21xx.h>           
#include "LCD.h"               
#include "KPM.h"             
#include "lcd_defines.h"     
#include "kpm_defines.h"      
#include "rtc.h"              
#include "interrupt.h"        

// Status flags
int flag=0,flag1=0,flag3=0,flag4=0,s=0;

// Store key, medicine times, current time and date
char key,m1[]="00:00:00",m2[]="00:00:00",m3[]="00:00:00",time[]="00:00:00",date[]="00/00/0000";

// External day names
extern char dayLUT[][4];

// Store day value
int day;

// Alert timer and previous second
int alert_timer = 0, last_second=0;

// Store last triggered minute
int last_triggered_minute = -1;


// Main function
int main()
{
	
	// Set buzzer pin as output
	IODIR0|=1<<BUZZER;

	// Initialize keypad
	InitKPM();

	// Initialize LCD
	InitLCD();

	// Initialize RTC
	RTC_Init();

	// Initialize interrupts
	Init_intrrupt();

	// Turn ON LCD
	CmdLCD(D_ON);

	// Run continuously
	while(1)
	{
		// Read current RTC time
		GetRTCTimeInfo(time);
		
		// Check menu flag
		if(flag)
		{
			// Set menu active flag
			flag1=1;

			// Turn ON LCD cursor and blink
			CmdLCD(D_ON_C_BLK);

			// Open configuration menu
			configure();

			// Clear menu active flag
			flag1=0;

			// Clear LCD
			CmdLCD(CLR);

			// Turn ON LCD
			CmdLCD(D_ON);
		}
					
		// Get tens digit of seconds
		s=(time[6]-'0')*10;

		// Get units digit of seconds
		s+=(time[7]-'0');
				
					
		// Check second change
		if ( s!= last_second)
		{
			// Store current second
			last_second = s;

			// Check alert timer
			if (alert_timer > 0)
			{
				// Decrease alert timer
				alert_timer--;

				// Check timer completed
				if(alert_timer == 0)

					// Reset triggered minute
					last_triggered_minute=-1;

				// Set LCD clear flag
				flag4=1;
			}
		}


		// Check medicine alert flag
		if (flag3)
		{
			// Check new minute
			if (MIN != last_triggered_minute)
			{
				// Compare medicine times with current time
				if ((check(m1, time)) || (check(m2, time)) || (check(m3, time)))
				{
					// Check alert timer is zero
					if(  alert_timer==0)
					{
						// Set alert timer
						alert_timer = 60;

						// Store triggered minute
						last_triggered_minute = MIN;
					}
				}
			}
		}


		// Check active reminder minute
		if ((last_triggered_minute == MIN))
		{
			// Move cursor to first line
			CmdLCD(GOTO_L1_POSN0);

			// Display medicine alert
			StrLCD((signed char*)"TAKE MEDICINE!  ");

			// Move cursor to second line
			CmdLCD(GOTO_L2_POSN0);

			// Display reminder text
			StrLCD((signed char*)"Rem: ");

			// Display alert timer
			intLCD(alert_timer);

			// Display seconds left
			StrLCD((signed char*)" sec left    ");
    
			// Turn ON buzzer
			IOSET0 = 1 << BUZZER;
		}
		else
		{
			// Turn OFF buzzer
			IOCLR0 = 1 << BUZZER;
    
			// Check LCD clear flag
			if (flag4)
			{
				// Clear LCD
				CmdLCD(CLR);

				// Clear LCD flag
				flag4 = 0;
			}

			// Read current RTC time
			GetRTCTimeInfo(time);

			// Move cursor to first line
			CmdLCD(GOTO_L1_POSN0);

			// Display current time
			StrLCD((signed char*)time);
    
			// Read current day
			GetRTCDay(&day);

			// Set day display position
			CmdLCD(GOTO_L1_POSN0 + 10);

			// Display day name
			StrLCD((signed char*)dayLUT[day]);

			// Read current date
			GetRTCDateInfo(date);

			// Move cursor to second line
			CmdLCD(GOTO_L2_POSN0);

			// Display current date
			StrLCD((signed char*)date);
		}
	}
}
