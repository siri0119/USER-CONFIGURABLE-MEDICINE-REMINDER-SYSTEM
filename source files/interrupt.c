// ==================================================================================
// FILE: interrupt.c
// DESCRIPTION: Handles hardware interrupts, RTC/Medicine configurations, and UI logic
// ==================================================================================

#include "interrupt.h"        // Interrupt header file
#include <lpc21xx.h>          // LPC21xx register definitions
#include "lcd_defines.h"      // LCD definitions
#include "LCD.h"              // LCD functions
#include "rtc.h"              // RTC functions
#include "KPM.h"              // Keypad functions

unsigned int year_val;        // Stores year value
char dayLUT[][4] = {"SUN","MON","TUE","WED","THU","FRI","SAT"}; // Stores day names
unsigned int cnt;             // Global counter variable
extern int day, last_triggered_minute; // External day and last triggered minute variables
int i, temp;                  // Variables used for loop and temporary value
extern int flag, flag1, flag3, flag4; // External flag variables
extern char key, m1[], m2[], m3[], time[], date[]; // External key, medicine, time and date variables

extern int alert_timer;       // External alert timer variable

int check(char* p, char* q)   // Compares two time strings
{
	int i = 0;                  // Start index from 0
	while(i < 5)                // Check first 5 time characters
	{
		if(p[i] != q[i])          // Check both characters
			return 0;               // Return 0 if characters do not match
		i++;                      // Move to next character
	}
	return 1;                   // Return 1 if all characters match
}

void Init_intrrupt(void)      // Initialize interrupts
{
	// Configure P0.1 and P0.3 as EINT0 and EINT1
	// Input pins
	// Clear bit pairs without affecting other bits
	PINSEL0 &= ((u32)~3 << 2) | ((u32)~3 << 4); // Clear pin function bits
	
	// Select EINT0 and EINT1 pin functions
	PINSEL0 |= EINT0_INPUT_PIN | EINT1_INPUT_PIN;
	
	// Configure VIC
	// Enable EINT0 and EINT1 channels
	VICIntEnable = 1 << EINT0_VIC_CHN0 | 1 << EINT1_VIC_CHN0;
	
	// Configure EINT0 as vectored IRQ
	VICVectCntl1 = (1 << 5) | EINT0_VIC_CHN0;
	
	// Store EINT0 ISR address
	VICVectAddr1 = (u32)eint0_isr;

	// Configure EINT1 as vectored IRQ
	VICVectCntl0 = (1 << 5) | EINT1_VIC_CHN0;
	
	// Store EINT1 ISR address
	VICVectAddr0 = (u32)eint1_isr;

	// Configure EINT0 and EINT1
	// Set as edge triggered interrupts
	EXTMODE = ((1 << 1) | (1 << 0));
	
	// EINT0 and EINT1 are falling edge triggered
}

void eint0_isr(void) __irq   // EINT0 interrupt service routine
{
	// Clear EINT0 interrupt status
	EXTINT = 1 << 0;
	
	// Clear VIC interrupt status
	VICVectAddr = 0;
	
	if(((IOPIN0 >> BUZZER) & 1) == 0) // Check if buzzer is OFF
	{
		flag = 1;                 // Set configuration flag
		IOCLR0 = 1 << 1;          // Clear P0.1
	}
}

void eint1_isr(void) __irq   // EINT1 interrupt service routine
{
	EXTINT = 1 << 1;            // Clear EINT1 interrupt status
	VICVectAddr = 0;            // Clear VIC interrupt status
	
	if(((IOPIN0 >> BUZZER) & 1) == 1) // Check if buzzer is ON
	{
		IOCLR0 = 1 << BUZZER;     // Turn OFF buzzer
		alert_timer = 0;          // Reset alert timer
		
		CmdLCD(CLR);              // Clear LCD
		StrLCD((signed char*)"Medicine Taken"); // Display medicine taken message
		delay_ms(1000);           // Wait for 1 second
		CmdLCD(CLR);              // Clear LCD
		
		flag4 = 1;                // Set flag4
		flag3 = 0;                // Clear flag3
		last_triggered_minute = -1; // Reset last triggered minute
	}
	else if(flag1 == 1)         // Check configuration mode
	{
		// Do nothing if already in configuration menu
	}
	else
	{
		CmdLCD(CLR);              // Clear LCD
		StrLCD((signed char*)"No Remindiers"); // Display no reminders message
		delay_ms(1000);           // Wait for 1 second
		CmdLCD(CLR);              // Clear LCD
	}
}

void menu2(void)                // RTC edit menu
{
	while(1)                    // Repeat menu until exit
	{
		CmdLCD(CLR);              // Clear LCD
		CmdLCD(GOTO_L1_POSN0);    // Move cursor to first line
		StrLCD((signed char*)"1.RTC Time 2.Dy"); // Display first menu line
		CmdLCD(GOTO_L2_POSN0);    // Move cursor to second line
		StrLCD((signed char*)"3.RTC Date 4.Q"); // Display second menu line
		
		key = KeyScan();          // Read key from keypad
		
		switch(key)               // Check selected key
		{
			case '1':             // RTC time editing
				while(1)
				{
					CmdLCD(CLR);      // Clear LCD
					CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
					StrLCD((signed char*)time); // Display current time
					CmdLCD(GOTO_L1_POSN0 + 11); // Move cursor position
					StrLCD((signed char*)"4.exit"); // Display exit option
					CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
					StrLCD((signed char*)"1.hr 2.min 3.sec"); // Display time edit options
					
					key = KeyScan();  // Read key from keypad
					
					switch(key)
					{
						// Hour setting
						case '1':
							flag3 = 1;    // Set flag3
							
						g: CmdLCD(CLR);  // Clear LCD
							CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
							StrLCD((signed char*)time); // Display current time
							CmdLCD(GOTO_L1_POSN0); // Move cursor to hour position
							
							time[0] = KeyScan(); // Read first hour digit
							CharLCD(time[0]); // Display entered digit
							
							if(time[0] == 'C') // Check clear key
							{
								time[0] = '0'; // Reset first hour digit
								goto g;         // Enter hour again
							}
							else if(time[0] >= '0' && time[0] <= '2') // Check first hour digit
							{
							h2: CmdLCD(CLR); // Clear LCD
								CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
								StrLCD((signed char*)time); // Display current time
								CmdLCD(GOTO_L1_POSN0 + 1); // Move cursor to second hour digit
								
								time[1] = KeyScan(); // Read second hour digit
								
								if(time[1] == 'C') // Check clear key
								{
									time[0] = '0'; // Reset first hour digit
									time[1] = '0'; // Reset second hour digit
									goto g;         // Enter hour again
								}
								else if(((time[0] >= '0' && time[0] <= '1') && (time[1] >= '0' && time[1] <= '9'))
									|| (time[0] == '2' && (time[1] >= '0') && (time[1] <= '3'))) // Check valid hour
								{
									CharLCD(time[1]); // Display second hour digit
									
									// Set RTC time
									SetRTCTimeInfo(time);
								}
								else
								{
									time[1] = '0'; // Reset second hour digit
									CmdLCD(CLR); // Clear LCD
									CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
									StrLCD((signed char*)"Invalid Hour"); // Display invalid hour
									CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
									StrLCD((signed char*)"Try Again"); // Display try again
									delay_ms(100); // Wait
									goto h2; // Enter second hour digit again
								}
							}
							else
							{
								time[0] = '0'; // Reset first hour digit
								CmdLCD(CLR); // Clear LCD
								CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
								StrLCD((signed char*)"Invalid Hour"); // Display invalid hour
								CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
								StrLCD((signed char*)"Try Again"); // Display try again
								delay_ms(100); // Wait
								goto g; // Enter hour again
							}
							break;
						
						// Minute setting
						case '2':
							flag3 = 1;    // Set flag3
							
						h: CmdLCD(CLR);  // Clear LCD
							CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
							StrLCD((signed char*)time); // Display current time
							CmdLCD(GOTO_L1_POSN0 + 3); // Move cursor to minute position
							
							time[3] = KeyScan(); // Read first minute digit
							CharLCD(time[3]); // Display entered digit
							
							if(time[3] == 'C') // Check clear key
							{
								time[3] = '0'; // Reset first minute digit
								goto h;         // Enter minute again
							}
							else if(time[3] >= '0' && time[3] <= '5') // Check first minute digit
							{
							min: CmdLCD(CLR); // Clear LCD
								CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
								StrLCD((signed char*)time); // Display current time
								CmdLCD(GOTO_L1_POSN0 + 4); // Move cursor to second minute digit
								
								time[4] = KeyScan(); // Read second minute digit
								
								if(time[4] == 'C') // Check clear key
								{
									time[3] = '0'; // Reset first minute digit
									time[4] = '0'; // Reset second minute digit
									goto h;         // Enter minute again
								}
								else if(time[4] >= '0' && time[4] <= '9') // Check second minute digit
								{
									CharLCD(time[4]); // Display second minute digit
									SetRTCTimeInfo(time); // Set RTC time
								}
								else
								{
									time[4] = '0'; // Reset second minute digit
									CmdLCD(CLR); // Clear LCD
									CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
									StrLCD((signed char*)"Invalid Minute"); // Display invalid minute
									CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
									StrLCD((signed char*)"Try Again"); // Display try again
									delay_ms(100); // Wait
									goto min; // Enter second minute digit again
								}
							}
							else
							{
								time[3] = '0'; // Reset first minute digit
								CmdLCD(CLR); // Clear LCD
								CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
								StrLCD((signed char*)"Invalid Minute"); // Display invalid minute
								CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
								StrLCD((signed char*)"Try Again"); // Display try again
								delay_ms(100); // Wait
								goto h; // Enter minute again
							}
							break;
						// seconds
						case '3':       // Set RTC seconds
							flag3 = 1;    // Set flag3
						n: CmdLCD(CLR);  // Clear LCD
							CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
							StrLCD((signed char*)time); // Display current time
							CmdLCD(GOTO_L1_POSN0 + 6); // Move cursor to first second digit
							time[6] = KeyScan(); // Read first second digit
							CharLCD(time[6]); // Display entered digit
							if(time[6] == 'C') // Check clear key
							{
								time[6] = '0'; // Reset first second digit
								goto n;     // Enter seconds again
							}
							else if(time[6] >= '0' && time[6] <= '5') // Check first second digit
							{
							sec: CmdLCD(CLR); // Clear LCD
								CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
								StrLCD((signed char*)time); // Display current time
								CmdLCD(GOTO_L1_POSN0 + 7); // Move cursor to second digit
								time[7] = KeyScan(); // Read second digit
								if(time[7] == 'C') // Check clear key
								{
									time[6] = '0'; // Reset first digit
									time[7] = '0'; // Reset second digit
									goto n; // Enter seconds again
								}
								else if(time[7] >= '0' && time[7] <= '9') // Check valid digit
								{
									CharLCD(time[7]); // Display second digit
									SetRTCTimeInfo(time); // Update RTC time
								}
								else
								{
									time[7] = '0'; // Reset second digit
									CmdLCD(CLR); // Clear LCD
									CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
									StrLCD((signed char*)"Invalid Second"); // Display invalid second
									CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
									StrLCD((signed char*)"Try Again"); // Display try again
									delay_ms(100); // Wait
									goto sec; // Enter second digit again
								}
							}
							else
							{
								time[6] = '0'; // Reset first second digit
								CmdLCD(CLR); // Clear LCD
								CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
								StrLCD((signed char*)"Invalid Second"); // Display invalid second
								CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
								StrLCD((signed char*)"Try Again"); // Display try again
								delay_ms(100); // Wait
								goto n;     // Enter seconds again
							}
							break;        // Exit seconds case
						case '4':       // Exit time menu
							break;
						default:        // Invalid key
							CmdLCD(CLR);  // Clear LCD
							CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
							StrLCD((signed char*)"Invalid Input"); // Display invalid input
							CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
							StrLCD((signed char*)"Try Again"); // Display try again
							delay_ms(1000); // Wait for 1 second
							break;
					}
					if(key == '4')    // Check exit key
						break;          // Exit time edit loop
				}
				break;
						
			// RTC day
			case '2':               // RTC day editing
				while(1)
				{
					CmdLCD(CLR);      // Clear LCD
					CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
					StrLCD((signed char*)"0.S 1.M 2.TU 3.W"); // Display day options
					CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
					StrLCD((signed char*)"4.T 5.F 6.SA 7.Q"); // Display remaining day options
					key = KeyScan();  // Read key from keypad
					if(!(key >= '0' && key <= '7')) // Check valid day input
					{
						CmdLCD(CLR);    // Clear LCD
						CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
						StrLCD((signed char*)"Invalid Day"); // Display invalid day
						CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
						StrLCD((signed char*)"Try Again"); // Display try again
						delay_ms(100);  // Wait
					}
					else if(key == '7') // Check exit key
					{
						break;          // Exit day menu
					}
					else
					{
						SetRTCDay((int)key - '0'); // Set RTC day
						CmdLCD(CLR);    // Clear LCD
						CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
						StrLCD((signed char*)"Day saved!"); // Display saved message
						delay_ms(500);  // Wait
					}
				}
				break;
				
			// RTC date
			case '3':               // RTC date editing
				while(1)
				{
					CmdLCD(CLR);      // Clear LCD
					CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
					StrLCD((signed char*)date); // Display current date
					CmdLCD(GOTO_L1_POSN0 + 11); // Move cursor position
					StrLCD((signed char*)"4.exit"); // Display exit option
					CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
					StrLCD((signed char*)"1.dt 2.mon 3.yr"); // Display date edit options
					key = KeyScan();  // Read key from keypad
					switch(key)
					{
						// date
						case '1':       // Set date
						i: CmdLCD(CLR);  // Clear LCD
							CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
							StrLCD((signed char*)date); // Display current date
							CmdLCD(GOTO_L1_POSN0); // Move cursor to date position
							date[0] = KeyScan(); // Read first date digit
							CharLCD(date[0]); // Display entered digit
							if(date[0] == 'C') // Check clear key
							{
								date[0] = '0'; // Reset first date digit
								goto i;     // Enter date again
							}
							else if(date[0] >= '0' && date[0] <= '3') // Check first date digit
							{
							date: CmdLCD(CLR); // Clear LCD
								CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
								StrLCD((signed char*)date); // Display current date
								CmdLCD(GOTO_L1_POSN0 + 1); // Move cursor to second date digit
								date[1] = KeyScan(); // Read second date digit
								if(date[1] == 'C') // Check clear key
								{
									date[0] = '0'; // Reset first date digit
									date[1] = '0'; // Reset second date digit
									goto i; // Enter date again
								}
								else if((date[0] == '0' && (date[1] >= '1' && date[1] <= '9'))
									|| ((date[0] >= '1' && date[0] <= '2') && (date[1] >= '0' && date[1] <= '9'))
									|| (date[0] == '3' && (date[1] >= '0') && date[1] <= '1')) // Check valid date
								{
									CharLCD(date[1]); // Display second date digit
									SetRTCDateInfo(date); // Update RTC date
								}
								else
								{
									date[1] = '0'; // Reset second date digit
									CmdLCD(CLR); // Clear LCD
									CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
									StrLCD((signed char*)"Invalid Date"); // Display invalid date
									CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
									StrLCD((signed char*)"Try Again"); // Display try again
									delay_ms(100); // Wait
									goto date; // Enter second date digit again
								}
							}
							else
							{
								date[0] = '0'; // Reset first date digit
								CmdLCD(CLR); // Clear LCD
								CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
								StrLCD((signed char*)"Invalid Date"); // Display invalid date
								CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
								StrLCD((signed char*)"Try Again"); // Display try again
								delay_ms(100); // Wait
								goto i;     // Enter date again
							}
							break;
						
						// month
						case '2':       // Set month
						j: CmdLCD(CLR);  // Clear LCD
							CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
							StrLCD((signed char*)date); // Display current date
							CmdLCD(GOTO_L1_POSN0 + 3); // Move cursor to month position
							date[3] = KeyScan(); // Read first month digit
							CharLCD(date[3]); // Display entered digit
							if(date[3] == 'C') // Check clear key
							{
								date[3] = '0'; // Reset first month digit
								goto i;     // Enter date again
							}
							else if(date[3] >= '0' && date[3] <= '1') // Check first month digit
							{
							month: CmdLCD(CLR); // Clear LCD
								CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
								StrLCD((signed char*)date); // Display current date
								CmdLCD(GOTO_L1_POSN0 + 4); // Move cursor to second month digit
								date[4] = KeyScan(); // Read second month digit
								if(date[4] == 'C') // Check clear key
								{
									date[3] = '0'; // Reset first month digit
									date[4] = '0'; // Reset second month digit
									goto j; // Enter month again
								}
								else if((date[3] == '0' && (date[4] >= '1' && date[4] <= '9')) ||
									(date[3] == '1' && (date[4] >= '0' && date[4] <= '2'))) // Check valid month
								{
									CharLCD(date[1]); // Display digit
									SetRTCDateInfo(date); // Update RTC date
								}
								else
								{
									date[4] = '0'; // Reset second month digit
									CmdLCD(CLR); // Clear LCD
									CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
									StrLCD((signed char*)"Invalid month"); // Display invalid month
									CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
									StrLCD((signed char*)"Try Again"); // Display try again
									delay_ms(100); // Wait
									goto month; // Enter second month digit again
								}
							}
							else
							{
								date[3] = '0'; // Reset first month digit
								CmdLCD(CLR); // Clear LCD
								CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
								StrLCD((signed char*)"Invalid month"); // Display invalid month
								CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
								StrLCD((signed char*)"Try Again"); // Display try again
								delay_ms(100); // Wait
								goto j;     // Enter month again
							}
							break;
						
						// year
						case '3':       // Set year
						k: CmdLCD(CLR);  // Clear LCD
							StrLCD((signed char*)"Year(0-4095):"); // Display year range
							CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
														// Collect 4 digits and check each one for "numeric only"
							for(i = 0; i < 4; i++) // Read 4 digits
							{
								temp = KeyScan(); // Read key from keypad
								
								// Check if key is a digit
								if(temp >= '0' && temp <= '9') // Check numeric key
								{
									date[6 + i] = temp; // Store digit
									CharLCD(temp);   // Display digit
								}
								// Check clear key
								else if(temp == 'C')
								{
									goto k;         // Enter year again
								}
								// Check invalid key
								else
								{
									CmdLCD(CLR);    // Clear LCD
									StrLCD((signed char*)"Invalid Key!"); // Display invalid key
									delay_ms(1000); // Wait for 1 second
									goto k;         // Enter year again
								}
							}

							// Convert 4 digits into year value
							year_val = ((date[6] - '0') * 1000) + 
							           ((date[7] - '0') * 100) + 
							           ((date[8] - '0') * 10) + 
							           (date[9] - '0');

							if(year_val > 4095) // Check maximum year value
							{
								CmdLCD(CLR);      // Clear LCD
								StrLCD((signed char*)"Max is 4095!"); // Display maximum year message
								delay_ms(1000);   // Wait for 1 second
								goto k;           // Enter year again
							}
							else
							{
								SetRTCDateInfo(date); // Set RTC date
								StrLCD((signed char*)" - Saved!"); // Display saved message
								delay_ms(1000);   // Wait for 1 second
							}
							break;
						case '4':       // Exit date menu
							break;
						default:        // Invalid input
							CmdLCD(CLR);  // Clear LCD
							CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
							StrLCD((signed char*)"Invalid Input"); // Display invalid input
							CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
							StrLCD((signed char*)"Try Again"); // Display try again
							delay_ms(1000); // Wait for 1 second
							break;
					}
					if(key == '4')    // Check exit key
						break;
				}
				break;
						
			case '4':               // Exit RTC menu
				break;
			default:                // Invalid input
				CmdLCD(CLR);        // Clear LCD
				CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
				StrLCD((signed char*)"Invalid Input"); // Display invalid input
				CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
				StrLCD((signed char*)"Try Again"); // Display try again
				delay_ms(1000);     // Wait for 1 second
				break;
		}
		if(key == '4')              // Check exit key
			break;
	}
}

void configure(void)            // Main configuration menu
{
	flag = 0;                     // Reset flag
	while(1)                      // Repeat configuration menu
	{
		/*menu-1
		1.time,date,day editing
		2.New Entry or update medicine details and Remainders
		3.exit menu and display real time clock*/
		CmdLCD(CLR);                // Clear LCD
		CmdLCD(GOTO_L1_POSN0);      // Move cursor to first line
		StrLCD((signed char*)"1.RTC Edit 3.Q"); // Display RTC edit and exit options
		CmdLCD(GOTO_L2_POSN0);      // Move cursor to second line
		StrLCD((signed char*)"2.Medicine Edit"); // Display medicine edit option
		key = KeyScan();            // Read key from keypad
		// Run process based on key
		
		switch(key)
		{
			/*menu 2
			1.hour,2.min,3.sec,4.day,5.date,6.month,7.year,8.quit*/
			case '1':                 // RTC edit option
				menu2();                // Open RTC edit menu
				break;
			
			/*menu 3
			1.1st medicine details 2.2nd medicine details 3.3rd medicine details*/
			case '2':                 // Medicine edit option
				while(1)
				{
					CmdLCD(CLR);        // Clear LCD
					CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
					StrLCD((signed char*)"1.Med1 3.Med3"); // Display medicine 1 and 3 options
					CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
					StrLCD((signed char*)"2.Med2 4.Exit"); // Display medicine 2 and exit options
					key = KeyScan();    // Read key from keypad
					switch(key)
					{
						// for medicine 1
						case '1':         // Edit medicine 1 time
							while(1)
							{
								CmdLCD(CLR); // Clear LCD
								CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
								StrLCD((signed char*)m1); // Display medicine 1 time
								CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
								StrLCD((signed char*)"1.hr 2.min 3.Q"); // Display edit options
								key = KeyScan(); // Read key from keypad
								switch(key)
								{
									// for hour set
									case '1': // Set medicine 1 hour
									a: CmdLCD(CLR); // Clear LCD
										CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
										StrLCD((signed char*)m1); // Display medicine 1 time
										CmdLCD(GOTO_L1_POSN0); // Move cursor to hour position
										m1[0] = KeyScan(); // Read first hour digit
										CharLCD(m1[0]); // Display digit
										if(m1[0] == 'C') // Check clear key
										{
											m1[0] = '0'; // Reset first hour digit
											goto a; // Enter hour again
										}
										else if(m1[0] >= '0' && m1[0] <= '2') // Check first hour digit
										{
										m1h: CmdLCD(CLR); // Clear LCD
											CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
											StrLCD((signed char*)m1); // Display medicine 1 time
											CmdLCD(GOTO_L1_POSN0 + 1); // Move cursor to second hour digit
											m1[1] = KeyScan(); // Read second hour digit
											if(m1[1] == 'C') // Check clear key
											{
												m1[0] = '0'; // Reset first hour digit
												m1[1] = '0'; // Reset second hour digit
												goto a; // Enter hour again
											}
											else if(((m1[0] >= '0' && m1[0] <= '1') && (m1[1] >= '0' && m1[1] <= '9'))
												|| (m1[0] == '2' && (m1[1] >= '0') && (m1[1] <= '3'))) // Check valid hour
											{
												CharLCD(m1[1]); // Display second hour digit
											}
											else
											{
												m1[1] = '0'; // Reset second hour digit
												CmdLCD(CLR); // Clear LCD
												CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
												StrLCD((signed char*)"Invalid Hour"); // Display invalid hour
												CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
												StrLCD((signed char*)"Try Again"); // Display try again
												delay_ms(100); // Wait
												goto m1h; // Enter second hour digit again
											}
										}
										else
										{
											m1[0] = '0'; // Reset first hour digit
											CmdLCD(CLR); // Clear LCD
											CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
											StrLCD((signed char*)"Invalid Hour"); // Display invalid hour
											CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
											StrLCD((signed char*)"Try Again"); // Display try again
											delay_ms(100); // Wait
											goto a; // Enter hour again
										}	
										break;

									// for min set
									case '2': // Set medicine 1 minute
									b: CmdLCD(CLR); // Clear LCD
										CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
										StrLCD((signed char*)m1); // Display medicine 1 time
										CmdLCD(GOTO_L1_POSN0 + 3); // Move cursor to first minute digit
										m1[3] = KeyScan(); // Read first minute digit
										CharLCD(m1[3]); // Display digit
										if(m1[3] == 'C') // Check clear key
										{
											m1[3] = '0'; // Reset first minute digit
											goto b; // Enter minute again
										}
										else if(m1[3] >= '0' && m1[3] <= '5') // Check first minute digit
										{
										m1m: CmdLCD(CLR); // Clear LCD
											CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
											StrLCD((signed char*)m1); // Display medicine 1 time
											CmdLCD(GOTO_L1_POSN0 + 4); // Move cursor to second minute digit
											m1[4] = KeyScan(); // Read second minute digit
											if(m1[4] == 'C') // Check clear key
											{
												m1[3] = '0'; // Reset first minute digit
												m1[4] = '0'; // Reset second minute digit
												goto b; // Enter minute again
											}
											else if(m1[4] >= '0' && m1[4] <= '9') // Check second minute digit
											{
												CharLCD(m1[4]); // Display second minute digit
											}
											else
											{
												m1[4] = '0'; // Reset second minute digit
												CmdLCD(CLR); // Clear LCD
												CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
												StrLCD((signed char*)"Invalid Minute"); // Display invalid minute
												CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
												StrLCD((signed char*)"Try Again"); // Display try again
												delay_ms(100); // Wait
												goto m1m; // Enter second minute digit again
											}
										}
										else
										{
											m1[3] = '0'; // Reset first minute digit
											CmdLCD(CLR); // Clear LCD
											CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
											StrLCD((signed char*)"Invalid Minute"); // Display invalid minute
											CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
											StrLCD((signed char*)"Try Again"); // Display try again
											delay_ms(100); // Wait
											goto b; // Enter minute again
										}
										break;
									
									case '3': // Exit medicine 1 edit
										break;
									
									default: // Invalid input
										CmdLCD(CLR); // Clear LCD
										CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
										StrLCD((signed char*)"Invalid Input"); // Display invalid input
										CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
										StrLCD((signed char*)"Try Again"); // Display try again
										delay_ms(1000); // Wait for 1 second
										break;
								}
								if(key == '3') // Check exit key
									break;
							}
							break;

						// for medicine 2
						case '2':         // Edit medicine 2 time
							while(1)
															{
								CmdLCD(CLR); // Clear LCD
								CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
								StrLCD((signed char*)m2); // Display medicine 2 time
								CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
								StrLCD((signed char*)"1.hr 2.min 3.Q"); // Display edit options
								key = KeyScan(); // Read key from keypad
								switch(key)
								{
									// for hour set
									case '1': // Set medicine 2 hour
									c: CmdLCD(CLR); // Clear LCD
										CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
										StrLCD((signed char*)m2); // Display medicine 2 time
										CmdLCD(GOTO_L1_POSN0); // Move cursor to hour position
										m2[0] = KeyScan(); // Read first hour digit
										CharLCD(m2[0]); // Display digit
										if(m2[0] == 'C') // Check clear key
										{
											m2[0] = '0'; // Reset first hour digit
											goto c; // Enter hour again
										}
										else if(m2[0] >= '0' && m2[0] <= '2') // Check first hour digit
										{
										m2h: CmdLCD(CLR); // Clear LCD
											CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
											StrLCD((signed char*)m2); // Display medicine 2 time
											CmdLCD(GOTO_L1_POSN0 + 1); // Move cursor to second hour digit
											m2[1] = KeyScan(); // Read second hour digit
											if(m2[1] == 'C') // Check clear key
											{
												m2[0] = '0'; // Reset first hour digit
												m2[1] = '0'; // Reset second hour digit
												goto c; // Enter hour again
											}
											else if(((m2[0] >= '0' && m2[0] <= '1') && (m2[1] >= '0' && m2[1] <= '9'))
												|| (m2[0] == '2' && (m2[1] >= '0') && (m2[1] <= '3'))) // Check valid hour
											{
												CharLCD(m2[1]); // Display second hour digit
											}
											else
											{
												m2[1] = '0'; // Reset second hour digit
												CmdLCD(CLR); // Clear LCD
												CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
												StrLCD((signed char*)"Invalid Hour"); // Display invalid hour
												CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
												StrLCD((signed char*)"Try Again"); // Display try again
												delay_ms(100); // Wait
												goto m2h; // Enter second hour digit again
											}
										}
										else
										{
											m2[0] = '0'; // Reset first hour digit
											CmdLCD(CLR); // Clear LCD
											CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
											StrLCD((signed char*)"Invalid Hour"); // Display invalid hour
											CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
											StrLCD((signed char*)"Try Again"); // Display try again
											delay_ms(100); // Wait
											goto c; // Enter hour again
										}	
										break;

									// for min set
									case '2': // Set medicine 2 minute
									d: CmdLCD(CLR); // Clear LCD
										CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
										StrLCD((signed char*)m2); // Display medicine 2 time
										CmdLCD(GOTO_L1_POSN0 + 3); // Move cursor to first minute digit
										m2[3] = KeyScan(); // Read first minute digit
										CharLCD(m2[3]); // Display digit
										if(m2[3] == 'C') // Check clear key
										{
											m2[3] = '0'; // Reset first minute digit
											goto d; // Enter minute again
										}
										else if(m2[3] >= '0' && m2[3] <= '5') // Check first minute digit
										{
										m2m: CmdLCD(CLR); // Clear LCD
											CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
											StrLCD((signed char*)m2); // Display medicine 2 time
											CmdLCD(GOTO_L1_POSN0 + 4); // Move cursor to second minute digit
											m2[4] = KeyScan(); // Read second minute digit
											if(m2[4] == 'C') // Check clear key
											{
												m2[3] = '0'; // Reset first minute digit
												m2[4] = '0'; // Reset second minute digit
												goto d; // Enter minute again
											}
											else if(m2[4] >= '0' && m2[4] <= '9') // Check second minute digit
											{
												CharLCD(m2[4]); // Display second minute digit
											}
											else
											{
												m2[4] = '0'; // Reset second minute digit
												CmdLCD(CLR); // Clear LCD
												CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
												StrLCD((signed char*)"Invalid Minute"); // Display invalid minute
												CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
												StrLCD((signed char*)"Try Again"); // Display try again
												delay_ms(100); // Wait
												goto m2m; // Enter second minute digit again
											}
										}
										else
										{
											m2[3] = '0'; // Reset first minute digit
											CmdLCD(CLR); // Clear LCD
											CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
											StrLCD((signed char*)"Invalid Minute"); // Display invalid minute
											CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
											StrLCD((signed char*)"Try Again"); // Display try again
											delay_ms(100); // Wait
											goto d; // Enter minute again
										}
										break;
									
									case '3': // Exit medicine 2 edit
										break;
									
									default: // Invalid input
										CmdLCD(CLR); // Clear LCD
										CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
										StrLCD((signed char*)"Invalid Input"); // Display invalid input
										CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
										StrLCD((signed char*)"Try Again"); // Display try again
										delay_ms(1000); // Wait for 1 second
										break;
								}
								if(key == '3') // Check exit key
									break;
							}
							break;

						// for medicine 3
						case '3': // Edit medicine 3 time
							while(1)
							{
								CmdLCD(CLR); // Clear LCD
								CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
								StrLCD((signed char*)m3); // Display medicine 3 time
								CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
								StrLCD((signed char*)"1.hr 2.min 3.Q"); // Display edit options
								key = KeyScan(); // Read key from keypad
								switch(key)
								{
									// for hour set
									case '1': // Set medicine 3 hour
									e: CmdLCD(CLR); // Clear LCD
										CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
										StrLCD((signed char*)m3); // Display medicine 3 time
										CmdLCD(GOTO_L1_POSN0); // Move cursor to hour position
										m3[0] = KeyScan(); // Read first hour digit
										CharLCD(m3[0]); // Display digit
										if(m3[0] == 'C') // Check clear key
										{
											m3[0] = '0'; // Reset first hour digit
											goto e; // Enter hour again
										}
										else if(m3[0] >= '0' && m3[0] <= '2') // Check first hour digit
										{
										m3h: CmdLCD(CLR); // Clear LCD
											CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
											StrLCD((signed char*)m3); // Display medicine 3 time
											CmdLCD(GOTO_L1_POSN0 + 1); // Move cursor to second hour digit
											m3[1] = KeyScan(); // Read second hour digit
											if(m3[1] == 'C') // Check clear key
											{
												m3[0] = '0'; // Reset first hour digit
												m3[1] = '0'; // Reset second hour digit
												goto e; // Enter hour again
											}
											else if(((m3[0] >= '0' && m3[0] <= '1') && (m3[1] >= '0' && m3[1] <= '9'))
												|| (m3[0] == '2' && (m3[1] >= '0') && (m3[1] <= '3'))) // Check valid hour
											{
												CharLCD(m3[1]); // Display second hour digit
											}
											else
											{
												m3[1] = '0'; // Reset second hour digit
												CmdLCD(CLR); // Clear LCD
												CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
												StrLCD((signed char*)"Invalid Hour"); // Display invalid hour
												CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
												StrLCD((signed char*)"Try again"); // Display try again
												delay_ms(100); // Wait
												goto m3h; // Enter second hour digit again
											}
										}
										else
										{
											m3[0] = '0'; // Reset first hour digit
											CmdLCD(CLR); // Clear LCD
											CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
											StrLCD((signed char*)"Invalid Hour"); // Display invalid hour
											CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
											StrLCD((signed char*)"Try again"); // Display try again
											delay_ms(100); // Wait
											goto e; // Enter hour again
										}	
										break;

									// for min set
									case '2': // Set medicine 3 minute
									f: CmdLCD(CLR); // Clear LCD
										CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
										StrLCD((signed char*)m3); // Display medicine 3 time
										CmdLCD(GOTO_L1_POSN0 + 3); // Move cursor to first minute digit
										m3[3] = KeyScan(); // Read first minute digit
										CharLCD(m3[3]); // Display digit
										if(m3[3] == 'C') // Check clear key
										{
											m3[3] = '0'; // Reset first minute digit
											goto f; // Enter minute again
										}
										else if(m3[3] >= '0' && m3[3] <= '5') // Check first minute digit
										{
										m3m: CmdLCD(CLR); // Clear LCD
											CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
											StrLCD((signed char*)m3); // Display medicine 3 time
											CmdLCD(GOTO_L1_POSN0 + 4); // Move cursor to second minute digit
											m3[4] = KeyScan(); // Read second minute digit
											if(m3[4] == 'C') // Check clear key
											{
												m3[3] = '0'; // Reset first minute digit
												m3[4] = '0'; // Reset second minute digit
												goto f; // Enter minute again
											}
											else if(m3[4] >= '0' && m3[4] <= '9') // Check second minute digit
											{
												CharLCD(m3[4]); // Display second minute digit
											}
											else  // Check invalid minute
											{
												m3[4] = '0'; // Reset second minute digit
												CmdLCD(CLR); // Clear LCD
												CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
												StrLCD((signed char*)"Invalid Minute"); // Display invalid minute
												CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
												StrLCD((signed char*)"Try again"); // Display try again
												delay_ms(100); // Wait
												goto m3m; // Enter second minute digit again
											}
										}
										else
										{
											m3[3] = '0'; // Reset first minute digit
											CmdLCD(CLR); // Clear LCD
											CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
											StrLCD((signed char*)"Invalid Minute"); // Display invalid minute
											CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
											StrLCD((signed char*)"Try again"); // Display try again
											delay_ms(100); // Wait
											goto f; // Enter minute again
										}
										break;
									
									case '3': // Exit medicine 3 edit
										break;
									
									default: // Invalid input
										CmdLCD(CLR); // Clear LCD
										CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
										StrLCD((signed char*)"Invalid Input"); // Display invalid input
										CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
										StrLCD((signed char*)"Try Again"); // Display try again
										delay_ms(1000); // Wait for 1 second
										break;
								}
								if(key == '3') // Check exit key
									break;
							}
							break;
				
						case '4': // Exit medicine menu
							break;
						default: // Invalid input
							CmdLCD(CLR); // Clear LCD
							CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
							StrLCD((signed char*)"Invalid Input"); // Display invalid input
							CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
							StrLCD((signed char*)"Try Again"); // Display try again
							delay_ms(1000); // Wait for 1 second
							break;
					}
					if(key == '4') // Check exit key
						break;
				}
				break;
		
			case '3': // Exit configuration menu
				break;
		
			default: // Invalid input
				CmdLCD(CLR); // Clear LCD
				CmdLCD(GOTO_L1_POSN0); // Move cursor to first line
				StrLCD((signed char*)"Invalid Input"); // Display invalid input
				CmdLCD(GOTO_L2_POSN0); // Move cursor to second line
				StrLCD((signed char*)"Try Again"); // Display try again
				delay_ms(1000); // Wait for 1 second
				break;
		}
	
		if(key == '3') // Check exit key
			break;
	}
}
											
