// ==================================================================================
// FILE: main1.c
// DESCRIPTION: Main application orchestration loop managing the real-time clock, Reminders, and UI
// ==================================================================================

#include<lpc21xx.h>           // Include core peripheral register map definitions for the NXP LPC21xx microcontrollers.
#include "LCD.h"              // Pull in high-level LCD function prototypes for printing text, numbers, and custom characters.
#include "KPM.h"              // Include keyboard matrix scanning function declarations to interface with buttons.
#include "lcd_defines.h"       // Incorporate LCD hardware pin layouts and display control command macros.
#include "kpm_defines.h"       // Incorporate matrix keypad pin row and column structural macro identifiers.
#include "rtc.h"              // Include hardware Real-Time Clock peripheral control routine definitions.
#include "interrupt.h"        // Include configuration parameters, ISR prototypes, and menu layout functions.

int flag=0,flag1=0,flag3=0,flag4=0,s=0; // Declare global status indicators tracking user menus, setup phases, alert resets, and running seconds.
char key,m1[]="00:00:00",m2[]="00:00:00",m3[]="00:00:00",time[]="00:00:00",date[]="00/00/0000"; // Allocate text strings storing active keystrokes, three medical reminders, clock time, and current date.
extern char dayLUT[][4];      // Reference the external look-up table containing shorthand names for the days of the week.
int day;                      // Declare an integer variable to hold the index value representing the current day.
// Define the alert duration (1 minute) // Technical description note detailing the intended active buzzer window length.
		int alert_timer = 0, last_second=0; // Define localized memory trackers counting down alert durations and storing the prior execution second.
int last_triggered_minute = -1; // Initialize tracking index to flag which specific minute has already fired an alarm sequence.

int main()                    // Entry boundary point launching the foundational software execution path.
{                             // Open core functional body block for the main embedded application.
	
	IODIR0|=1<<BUZZER;         // Configure the dedicated buzzer sound pin as a GPIO output line in Port 0 direction maps.
	InitKPM();                 // Call matrix keypad driver initialization routine to properly assert row pins.
	InitLCD();                 // Execute character LCD configuration steps to fix bus dimensions and clear memory.
	RTC_Init();                // Wake and enable internal real-time hardware clock oscillators.
	Init_intrrupt();           // Configure external edge triggers and vector slots inside the VIC driver.
	CmdLCD(D_ON);              // Issue instruction code activating the screen visual panel.
	while(1)                   // Instantiate the core non-blocking infinite super-loop polling system peripherals.
	{                          // Open processing loop block tracking scheduled tasks sequentially.
		  GetRTCTimeInfo(time);   // Retrieve current timestamp string from RTC registers and save inside the local array tracker.
		
					if(flag)           // Check state variable to determine if an external interrupt requested the interactive setup menu.
					{                  // Open isolated menu environment branch execution path.
						flag1=1;        // Raise system control flag to lock core interface rendering while in menu states.
						CmdLCD(D_ON_C_BLK); // Turn on a visible blinking block cursor to guide terminal keyboard configurations.
						configure();    // Execute the nested master setup interface routines to configure clock parameters and alerts.
						flag1=0;        // Lower system menu lock flag indicating terminal configuration is complete.
						CmdLCD(CLR);    // Wipe out residual menu character text footprints from display memory registers.
						CmdLCD(D_ON);   // Restore normal display rendering style by masking out the terminal cursor layout.
					}                  // Close configuration terminal execution control branches.
					
					s=(time[6]-'0')*10; // Extract tenth place digit character from seconds field and convert to integer representation.
					s+=(time[7]-'0');   // Extract ones place digit character from seconds field and accumulate absolute total seconds.
				
					
					//decrement the timer once per second // Document logic tracking the passage of seconds to execute downcounters.
					
					if ( s!= last_second) // Construct a conditional check comparing current second count against previous loop cycle.
					{                  // Open the localized once-per-second interval execution block.
						last_second = s; // Synchronize tracker to lock down actions until clock steps forward again.
							if (alert_timer > 0) // Test if the countdown medicine alarm active timer is currently ticking.
								{              // Begin countdown processing parameters modification path.
									alert_timer--; // Decrement running warning duration value by exactly one unit factor.
									if(alert_timer == 0) // Check if medical reminder window has expired down to zero.
										last_triggered_minute=-1; // Reset tracking index to ensure the same minute allows clean subsequent triggers.
									flag4=1;    // Assert flag alerting system that screen state change needs redrawing.
								}              // Terminate internal alert duration decrement checking blocks.
							//last_second = s; // Commented out redundant tracking variable synchronization operation.
					}                  // Close interval checking frame executing once per second change.

					// 1. Logic to TRIGGER the alert // Note marking block containing pattern identification tasks for matching schedules.
if (flag3) {                         // Verify if medical alert system has been successfully activated by settings menus.
    // Trigger only if it's the start of a new minute AND we haven't handled this minute yet // Note documenting double trigger safeguards.
    if (MIN != last_triggered_minute) { // Check if hardware clock minute register has moved beyond the active alert marker.
        if ((check(m1, time)) || (check(m2, time)) || (check(m3, time))) // Compare time strings against the three stored reminder values.
					{                  // Open context loop verifying availability of alert infrastructure.
          if(  alert_timer==0)    // Ensure no overlapping warning processes are currently occupying the system registers.
					{                  // Open alert assertion parameters load branch path.
					alert_timer = 60;  // Load medical duration register with exactly sixty seconds of warning time.
            last_triggered_minute = MIN; // Store active hardware minute index to lock out repeated triggers.
         }                     // Conclude alert loading initialization step frameworks.
			  }                     // Conclude string match verification loop actions.
     }                         // Conclude single-trigger confirmation tracking checks.
}                                    // Conclude enabled master alarm logic checking block.

// 2. Logic to EXECUTE the alert (Buzzer & LCD) // Section note introducing interface rendering profiles for alert states.
if ((last_triggered_minute == MIN))  // Evaluate if current system clock matches the locked medical alarm minute.
{                                    // Open high-priority UI rendering route for active reminders.
    // Handle LCD Alert           // Explanatory comment outlining text streaming for alerts.
    CmdLCD(GOTO_L1_POSN0);           // Target cursor destination directly to first index slot of row line one.
    StrLCD((signed char*)"TAKE MEDICINE!  "); // Render the critical text warning prompt across the primary row interface.
    CmdLCD(GOTO_L2_POSN0);           // Jump character tracking pointer to starting address slot of row line two.
    StrLCD((signed char*)"Rem: ");   // Display shorthand prefix label for the ticking reminder countdown metrics.
    intLCD(alert_timer);             // Format and output the running remaining integer seconds value on screen.
    StrLCD((signed char*)" sec left    "); // Print structural text trailing labels completing visual countdown block.
    
    IOSET0 = 1 << BUZZER;            // Set GPIO output pin high to drive current to physical buzzer alert device.
}                                    // Conclude warning execution code path block frameworks.
else                                 // Fall through to default operational states when no active reminder matches.
{                                    // Open standard dashboard runtime environment execution scope.
    // 3. Logic for IDLE state (Clock) // Technical divider label organizing baseline time keeping processes.
    IOCLR0 = 1 << BUZZER;            // Pull sound generator pin low to ensure transducer alert is completely quiet.
    
    // Clear the alert text once when the timer hits zero // Developer note highlighting cleanup routine after alert periods.
    if (flag4) {                     // Query screen change request variable to check if warning screen requires clearing.
        CmdLCD(CLR);                 // Execute full command sweep wiping out reminder text fragments from DDRAM registers.
        flag4 = 0;                   // Reset request variable back down to false to stop duplicate clearing loops.
    }                                // Conclude screen refresh verification conditional processes.

    // Normal Clock Display          // Note indicating the implementation of standard home clock views.
    GetRTCTimeInfo(time);            // Query internal clock storage structures to fill time array with absolute values.
    CmdLCD(GOTO_L1_POSN0);           // Direct LCD layout cursor back to starting address position of row one.
    StrLCD((signed char*)time);      // Stream completely formatted real-time hour, minute, and second tracking text string.
    
    GetRTCDay(&day);                 // Fetch numerical indexing representation for current week day out of RTC.
    CmdLCD(GOTO_L1_POSN0 + 10);      // Position display cursor exactly ten offsets in to cleanly anchor calendar labels.
    StrLCD((signed char*)dayLUT[day]); // Extract corresponding literal name text out of look-up dictionary and print.

    GetRTCDateInfo(date);            // Pull up current numeric day, month, and calendar year parameters from chip registers.
    CmdLCD(GOTO_L2_POSN0);           // Reset visual cursor line destination over to start position of row two.
    StrLCD((signed char*)date);      // Print the updated formatted calendar information string across the terminal layer.
}	                                 // Exit regular idle state clock rendering parameter loop block.
	}                                // Close loop traversal cycle tracking system polling patterns.
}                                    // Terminate main system layout environment function boundary.