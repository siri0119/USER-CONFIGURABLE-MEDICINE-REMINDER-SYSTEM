// ==================================================================================
// FILE: interrupt.c
// DESCRIPTION: Handles hardware interrupts, RTC/Medicine configurations, and UI logic
// ==================================================================================

#include "interrupt.h"        // Include custom definitions and function prototypes for interrupt operations
#include <lpc21xx.h>          // Include core register definition header for NXP LPC21xx Microcontrollers
#include "lcd_defines.h"      // Include hardware command configurations and memory mappings for the LCD
#include "LCD.h"              // Include high-level character, string, and control function prototypes for LCD
#include "rtc.h"              // Include peripheral driver prototypes for the internal Real-Time Clock
#include "KPM.h"              // Include driver prototypes for Keypad Matrix input scanning operations

unsigned int year_val;        // Define a global numerical storage variable to hold and validate calculated 4-digit years
char dayLUT[][4] = {"SUN","MON","TUE","WED","THU","FRI","SAT"}; // Define a 2D look-up table array mapping day indices to 3-letter strings
unsigned int cnt;             // Declare a global auxiliary integer counter variable for general usage
extern int day, last_triggered_minute; // Reference external integer variables tracking current day index and alarm block markers
int i, temp;                  // Declare local context variables used in iterative loops and transient data caching
extern int flag, flag1, flag3, flag4; // Link to external control status flags managed concurrently by main loop loops
extern char key, m1[], m2[], m3[], time[], date[]; // Pull external string buffers for inputs, reminders, time text, and dates

extern int alert_timer;       // Import an external active dynamic countdown variable tracking buzzer alarm runtime

int check(char* p, char* q)   // Define a comparison routing to match active time strings against target alarm limits
{
	int i = 0;                  // Initialize a local iteration offset pointer starting at index zero
	while(i < 5)                // Loop exactly 5 iterations to check hours and minutes characters ("HH:MM")
	{
		if(p[i] != q[i])          // Inspect structural equality between character elements at the matching offsets
			return 0;               // Immediately drop out and report a zero integer status if elements mismatch
		i++;                      // Increment the step index pointer to evaluate the following character array index
	}
	return 1;                   // Return success code value one confirming that all monitored values align perfectly
}

void Init_intrrupt(void)      // Construct a peripheral initialization block to establish interrupt routing properties
{
	// cfg p0.1,p0.3 pin as EINT0,EINT1
	// input pins
	// clr bit pair 2&3 & bit pair 4&5,
	// w/o affecting other bits
	PINSEL0 &= ((u32)~3 << 2) | ((u32)~3 << 4); // Clear functional selections for Pin 0.1 and Pin 0.3 via bitmasks
	// update bit2&3,bit4&5 for EINT0,EINT1
	// pin function
	PINSEL0 |= EINT0_INPUT_PIN | EINT1_INPUT_PIN; // Direct Pin 0.1 and Pin 0.3 routing registers to connect to EINT0 and EINT1 lines
	
	// cfg VIC peripheral/block
	// allow EINT0,EINT1 as irq type
	// VICIntSelect=0; // default
	// enable EINT0,EINT1 through channel
	VICIntEnable = 1 << EINT0_VIC_CHN0 | 1 << EINT1_VIC_CHN0; // Set explicit bits in the Vector Interrupt Controller to unmask external lines
	
	// Cfg EINT0 as v.irq with highest priority(0)
	VICVectCntl1 = (1 << 5) | EINT0_VIC_CHN0; // Bind channel 14 to Slot 1 while flipping hardware bit-5 to confirm slot activation
	// load eint0_isr address into LUT sfr
	VICVectAddr1 = (u32)eint0_isr; // Load the exact execution address of the EINT0 ISR routine into vector slot 1

	// Cfg EINT1 as v.irq with next
	// highest priority(1)
	VICVectCntl0 = (1 << 5) | EINT1_VIC_CHN0; // Bind channel 15 to Slot 0 while flipping hardware bit-5 to confirm slot activation
	// load eint1_isr address into LUT sfr
	VICVectAddr0 = (u32)eint1_isr; // Load the exact execution address of the EINT1 ISR routine into vector slot 0

	// Cfg EINT0,EINT1 via 
	// External Interrupts Peripheral	
	// Enable EINT0,EINT1
	// EXTINT=0;// default
	// Cfg EINT0,EINT1 as edge triggered interrupt
	EXTMODE = ((1 << 1) | (1 << 0)); // Configure External Interrupt Mode register to treat both paths as edge-sensitive switches
	// cfg EINT0,EINT1 as falling edge triggered
	// EXTPOLAR=0;// default
	
	// cfg status led pin for EINT0,EINT1
	// as gpio out
}

void eint0_isr(void) __irq   // Implement the Vector Interrupt Service routine invoked when External Pin EINT0 triggers
{
	// clear EINT0 Status in Ext Int Peripheral
	EXTINT = 1 << 0;            // Write a high bit to clear the latch tracking external hardware channel 0 status
	// clear EINT0 status in VIC peripheral
	VICVectAddr = 0;            // Write a clearing dummy zero to VIC register to signal endpoint sequence to prioritizing hardware
	if(((IOPIN0 >> BUZZER) & 1) == 0) // Query GPIO pin status state to check if the buzzer line is low
	{
		flag = 1;                 // Assert configuration request flag state to pull application state away into user menus
		IOCLR0 = 1 << 1;          // Issue a low logic write command to Port 0 bit line 1
	}
}

void eint1_isr(void) __irq   // Implement the Vector Interrupt Service routine invoked when External Pin EINT1 triggers
{
	EXTINT = 1 << 1;            // Clear the latch tracking external hardware channel 1 status flags
	VICVectAddr = 0;            // Clear the interrupt block tracking logic state within the hardware prioritizing module
	if(((IOPIN0 >> BUZZER) & 1) == 1) // Inspect line status register to check if the audio buzzer output is active
	{
		IOCLR0 = 1 << BUZZER;     // Drive the target audio output pin to a low state to immediately suppress the alarm buzzer
		alert_timer = 0;          // Clear the duration counter tracker to drop the alert window tracking process
		
		CmdLCD(CLR);              // Issue clear operation matrix byte across the transmission lines to empty display memory
		StrLCD((signed char*)"Medicine Taken"); // Render a success validation line layout to the character view matrix panel
		delay_ms(1000);           // Enter an execution stall period spanning one full second to keep the visual message visible
		CmdLCD(CLR);              // Blank display text fields again to return system layout view templates to normal states
		flag4 = 1;                // Set interface transition flag to prompt baseline loop tracking updates
		flag3 = 0;                // Turn off active trigger permissions until conditions clear out elsewhere
		last_triggered_minute = -1; // Reset active tracking history back to an index safely outside normal real clock ranges
	}
	else if(flag1 == 1)         // Alternate path check to evaluate if configuration state modes are already active
	{
		// Do nothing if already in configuration menu
	}
	else                        // Fallback block executing when the user impacts the switch with no alerts running
	{
		CmdLCD(CLR);              // Execute clear matrix instruction to sweep character fields clean from panel layout
		StrLCD((signed char*)"No Remindiers"); // Render alert message indicating lack of running time matches
		delay_ms(1000);           // Hold operations frozen for a span of one thousand milliseconds for read visibility
		CmdLCD(CLR);              // Clear display characters away to cleanly format screen space for clock update cycles
	}
}

void menu2(void)                // Construct secondary application menu interface loop handling individual RTC adjustments
{
	while(1)                    // Initialize a persistent user input polling loop blocking external main routines
	{
		CmdLCD(CLR);              // Wipe character contents off display rows to draw updated structural elements
		CmdLCD(GOTO_L1_POSN0);    // Relocate standard graphic layout text drawing pointer address to Start of Row 1
		StrLCD((signed char*)"1.RTC Time 2.Dy"); // Print primary system options on the current LCD line
		CmdLCD(GOTO_L2_POSN0);    // Relocate standard graphic layout text drawing pointer address to Start of Row 2
		StrLCD((signed char*)"3.RTC Date 4.Q"); // Print secondary system options on the current LCD line
		key = KeyScan();          // Execute keypad scanning block to track physical keyboard choices made by user
		
		switch(key)               // Evaluate returned matrix dynamic parameter value to navigate execution routing pathways
		{
			case '1':               // Selection branch intended to facilitate adjusting active time string contents
				while(1)            // Begin sub-loop layer managing explicit segment adjustments for Hours, Minutes, and Seconds
				{
					CmdLCD(CLR);      // Purge active liquid crystal rendering cells of residual string characters
					CmdLCD(GOTO_L1_POSN0); // Command the cursor position hardware to return to the origin index of Row 1
					StrLCD((signed char*)time); // Cast and write the active time structural representation formatting string
					CmdLCD(GOTO_L1_POSN0 + 11); // Offset internal rendering tracking position rightward by eleven steps
					StrLCD((signed char*)"4.exit"); // Write out boundary interface escape text menu item choice label
					CmdLCD(GOTO_L2_POSN0); // Move cursor down line tracking coordinates to point directly to Row 2 base
					StrLCD((signed char*)"1.hr 2.min 3.sec"); // Write segment modification options text layouts across display width
					key = KeyScan();  // Scan keyboard array interface again to read unit element selection index
					switch(key)       // Direct internal flow execution configurations down chosen unit segment adjust paths
					{
						// hour
						case '1':       // Branch segment assigned explicitly for modifying active Clock Hours tracking data
							flag3 = 1;    // Enable validation synchronization status flag tracker markers
						g: CmdLCD(CLR);  // Destination marker layout reference handling text recovery routines upon error states
							CmdLCD(GOTO_L1_POSN0); // Force text layout reference pointer location to Row 1 Start
							StrLCD((signed char*)time); // Paint the operational layout structural layout framework array string
							CmdLCD(GOTO_L1_POSN0); // Force focus orientation pointer index layout precisely back to character 0
							time[0] = KeyScan(); // Block task tracking loops until keypad returns a value to place at offset zero
							CharLCD(time[0]); // Echo user keyboard entry value directly down communication pathways into hardware views
							if(time[0] == 'C') // Verify if the operator tapped the dedicated matrix drop clear boundary option
							{
								time[0] = '0'; // Overwrite targeted cell coordinate value back into fallback character state
								goto g;     // Re-route operational loop track sequence backward up to label g
							}
							else if(time[0] >= '0' && time[0] <= '2') // Validate that hours first numeric index fits limits
							{
							h2: CmdLCD(CLR); // Frame baseline interface screen space layout grid structure arrays afresh
								CmdLCD(GOTO_L1_POSN0); // Snap alignment cursor state back up onto row index level boundary
								StrLCD((signed char*)time); // Print context text format view containing new adjustments
								CmdLCD(GOTO_L1_POSN0 + 1); // Move target point rightward to highlight unit digit coordinate field
								time[1] = KeyScan(); // Block for single-character digit key matrix selection to insert at offset 1
								if(time[1] == 'C') // Verify if the correction action override code character was flagged
								{
									time[0] = '0'; // Clear out hours first location data cells back to structural zero
									time[1] = '0'; // Clear out hours second location data cells back to structural zero
									goto g; // Loop system execution pointers backward back up onto primary element label entry
								}
								else if(((time[0] >= '0' && time[0] <= '1') && (time[1] >= '0' && time[1] <= '9'))
									|| (time[0] == '2' && (time[1] >= '0') && (time[1] <= '3'))) // Check for valid hours boundaries (00-23)
								{
									CharLCD(time[1]); // Render valid chosen symbol entry directly into display view line locations
									// Set the initial time (hours, minutes, seconds)
									SetRTCTimeInfo(time); // Push fully updated system string structure directly into local RTC hardware
								}
								else        // Execution node taking action if parameters violate formal hours time scales
								{
									time[1] = '0'; // Clear out faulty element state value cells back to reliable baseline zero
									CmdLCD(CLR); // Clear structural layouts off terminal face interface cells
									CmdLCD(GOTO_L1_POSN0); // Recenter operational display coordinate maps to Top Left position
									StrLCD((signed char*)"Invalid Hour"); // Notify user that inputted hour range falls outside standards
									CmdLCD(GOTO_L2_POSN0); // Shift focus indicator line markers down context layer structures
									StrLCD((signed char*)"Try Again"); // Direct user action patterns through standard retry strings
									delay_ms(100); // Stall configuration routine activity cycles momentarily
									goto h2; // Return routine track execution steps back to internal digit recovery label
								}
							}
							else          // Fallback tracking block triggered when the initial hour character fails tests
							{
								time[0] = '0'; // Revert top layer cell index mapping back to clean base character
								CmdLCD(CLR); // Clear active viewing screen cells to handle error updates cleanly
								CmdLCD(GOTO_L1_POSN0); // Re-establish graphic printing bounds parameters at origin marker points
								StrLCD((signed char*)"Invalid Hour"); // Display notice stating primary data choices failed validation bounds
								CmdLCD(GOTO_L2_POSN0); // Drop graphic cursor focus paths down to next character track lines
								StrLCD((signed char*)"Try Again"); // Present operational retry text blocks back to user view spaces
								delay_ms(100); // Stall dynamic calculation execution tracks briefly for sensory visibility
								goto g;     // Branch control flow tracks directly back to target marker label g
							}	
							break;        // Break outward from hour case routine paths back into time branch level
						
						// minute
						case '2':       // Branch segment assigned explicitly for modifying active Clock Minutes tracking data
							flag3 = 1;    // Enforce data confirmation checking guidelines parameters onto configurations
						h: CmdLCD(CLR);  // Reset display screen elements by pushing a clear message down to registers
							CmdLCD(GOTO_L1_POSN0); // Establish target display field alignment coordinates back to Row 1 start
							StrLCD((signed char*)time); // Display time variable data structures across the character pane surface
							CmdLCD(GOTO_L1_POSN0 + 3); // Advance focus alignment forward across the text array onto tens minute index
							time[3] = KeyScan(); // Capture target matrix index value selection to record at offset three
							CharLCD(time[3]); // Display standard numeric symbol mapping character values down to LCD grids
							if(time[3] == 'C') // Test array strings to determine if an abort command request was given
							{
								time[3] = '0'; // Revert target element memory register states back to base character zero
								goto h;     // Re-route processing logic flow tracks backward onto loop entry label h
							}
							else if(time[3] >= '0' && time[3] <= '5') // Verify the tens character fits standard hexagesimal scales (0-5)
							{
							min: CmdLCD(CLR); // Clean the active system displaying elements framework back to base states
								CmdLCD(GOTO_L1_POSN0); // Relocate textual view cursor alignment locations back to the row origin
								StrLCD((signed char*)time); // Reprint the underlying time layout contents to dynamic panel fields
								CmdLCD(GOTO_L1_POSN0 + 4); // Step cursor forward four blocks to target structural unit minute position
								time[4] = KeyScan(); // Read explicit key element assignment data mapping directly into offset four
								if(time[4] == 'C') // Evaluate if user intends to fall back and purge local parameters
								{
									time[3] = '0'; // Restore context column tracking index data values back to baseline
									time[4] = '0'; // Clear out unit slot structure fields back to structural baseline
									goto h; // Re-align loop pointers back to primary branch parameter label markers
								}
								else if(time[4] >= '0' && time[4] <= '9') // Confirm that unit digit inputs reside cleanly inside numerical values
								{
									CharLCD(time[4]); // Draw single character unit digit onto display structure array blocks
									SetRTCTimeInfo(time); // Update the core RTC register states to match new composite string maps
								}
								else        // Handle error responses if the inputted digit fails numerical ranges
								{
									time[4] = '0'; // Enforce fallback data definitions down onto active parameters cells
									CmdLCD(CLR); // Refresh displaying interfaces by throwing structural wipe controls down
									CmdLCD(GOTO_L1_POSN0); // Re-establish text processing locations back up on line entry thresholds
									StrLCD((signed char*)"Invalid Minute"); // Render confirmation alert string details indicating boundary error
									CmdLCD(GOTO_L2_POSN0); // Move graphic focus indicators down onto secondary row frameworks
									StrLCD((signed char*)"Try Again"); // Encourage interactive user retries by populating layout prompts
									delay_ms(100); // Block operation steps briefly to make data updates visual to eyes
									goto min; // Loop workflow operations down into local fallback minute logic paths
								}
							}
							else          // Triggered when initial minute values exceed maximum timing definitions
							{
								time[3] = '0'; // Set index memory location contents straight back to reliable zero base
								CmdLCD(CLR); // Flash clear command updates across character displaying module blocks
								CmdLCD(GOTO_L1_POSN0); // Align tracking focus limits back up with main line text fields
								StrLCD((signed char*)"Invalid Minute"); // Alert operators concerning incorrect minutes input structural constraints
								CmdLCD(GOTO_L2_POSN0); // Drop writing coordinates down onto secondary interface layer regions
								StrLCD((signed char*)"Try Again"); // Present structural user retry options down onto layout grids
								delay_ms(100); // Hold operational flow tracking suspended across short tracking gaps
								goto h;     // Loop operational configurations direct to step marker label reference h
							}
							break;        // Break cleanly out from minute handling cases back into parent structures
						
						// seconds
						case '3':       // Branch segment assigned explicitly for modifying active Clock Seconds tracking data
							flag3 = 1;    // Toggle background state flags to require clock logic updates later
						n: CmdLCD(CLR);  // Send standard display formatting wipes directly out through signal lines
							CmdLCD(GOTO_L1_POSN0); // Target row structural start alignments back at standard default offsets
							StrLCD((signed char*)time); // Render string structures holding baseline clock time text information
							CmdLCD(GOTO_L1_POSN0 + 6); // Advance cursor coordinates straight onto tens seconds position offset
							time[6] = KeyScan(); // Process user terminal keystrokes to place directly into index six fields
							CharLCD(time[6]); // Print out echo character symbols down to visual liquid crystal matrix
							if(time[6] == 'C') // Inspect control parameters to evaluate if user chose to cancel entry
							{
								time[6] = '0'; // Re-write structural default data values back inside data cells
								goto n;     // Re-route program flow sequences backward onto structural step label n
							}
							else if(time[6] >= '0' && time[6] <= '5') // Verify tens-of-seconds entry is less than or equal to five
							{
							sec: CmdLCD(CLR); // Sweep away graphic elements from the active character viewing window
								CmdLCD(GOTO_L1_POSN0); // Bring baseline text alignment pointers back up onto top line borders
								StrLCD((signed char*)time); // Render updated operational system time tracking strings out to panel
								CmdLCD(GOTO_L1_POSN0 + 7); // Shift cursor tracking boundaries directly onto unit seconds text index
								time[7] = KeyScan(); // Scan matrix elements to pull unit seconds options data down to index 7
								if(time[7] == 'C') // Test character configurations to evaluate if an abort signal is present
								{
									time[6] = '0'; // Clear tens field data values directly back down into baseline representations
									time[7] = '0'; // Clear unit field data values directly back down into baseline representations
									goto n; // Branch control execution tracks back to the primary seconds label baseline
								}
								else if(time[7] >= '0' && time[7] <= '9') // Enforce valid boundaries for standard seconds inputs (0-9)
								{
									CharLCD(time[7]); // Render active character layout data items out onto hardware blocks
									SetRTCTimeInfo(time); // Synchronize operational changes directly with real underlying RTC registers
								}
								else        // Process unexpected data entries falling outside valid numeric formats
								{
									time[7] = '0'; // Enforce safe baseline default symbols down inside memory matrices
									CmdLCD(CLR); // Direct display hardware modules to drop current layout structures
									CmdLCD(GOTO_L1_POSN0); // Move graphic rendering references back up onto baseline boundaries
									StrLCD((signed char*)"Invalid Second"); // Output contextual notifications describing execution constraint issues
									CmdLCD(GOTO_L2_POSN0); // Step cursor markers down onto lower text track layers
									StrLCD((signed char*)"Try Again"); // Prompt interface re-tries by placing system display strings
									delay_ms(100); // Stall standard computing tracks across short millisecond gaps
									goto sec; // Force execution loops back down to sub-layer unit tracking labels
								}
							}
							else          // Handle errors when the tens-of-seconds digit choice violates boundaries
							{
								time[6] = '0'; // Drive value fields directly back into safe structural default characters
								CmdLCD(CLR); // Wipe layout panels clean to allow new error data viewing updates
								CmdLCD(GOTO_L1_POSN0); // Point display target focus markers to standard origin tracking boundaries
								StrLCD((signed char*)"Invalid Second"); // Print error warnings stating seconds data choices are out of bounds
								CmdLCD(GOTO_L2_POSN0); // Drop graphic generation targets down onto lower interface frames
								StrLCD((signed char*)"Try Again"); // Repopulate dynamic interface paths with structural menu retry terms
								delay_ms(100); // Wait out short operational gaps to let operators digest warnings
								goto n;     // Transfer functional workflow paths backward up to step entry label n
							}
							break;        // Break away from active seconds processing routines back to parent layers
						case '4':       // User exit selection to leave the sub-menu layer
							break;        // Break out from the active time editing sub-switch execution track
						default:        // Fallback catch handles any keystrokes that do not match menu parameters
							CmdLCD(CLR);  // Clear visual panels completely to handle invalid option displays safely
							CmdLCD(GOTO_L1_POSN0); // Shift printing alignments back to upper-left interface markers
							StrLCD((signed char*)"Invalid Input"); // Write error line explaining chosen keys are unmapped options
							CmdLCD(GOTO_L2_POSN0); // Drop focus targets onto bottom display rows to finalize feedback
							StrLCD((signed char*)"Try Again"); // Render simple operational instruction blocks encouraging re-entry
							delay_ms(1000); // Hold messaging active across a duration window lasting one full second
							break;        // Break out from individual timing component select statements cleanly
					}
					if(key == '4')    // Explicit escape condition check evaluating if exit keys were flagged by users
						break;          // Break persistent inner time modifications loop loops completely
				}
				break;              // Escape execution tracks down through standard time configuration loops completely
						
			// RTC day
			case '2':               // Switch branch routing configurations to adjust structural day indexes
				while(1)            // Begin standard loop capture checking day assignment options selections
				{
					CmdLCD(CLR);      // Wipe rendering display slots before rendering new choices menus lists
					CmdLCD(GOTO_L1_POSN0); // Move graphic focus indicators up onto start row locations
					StrLCD((signed char*)"0.S 1.M 2.TU 3.W"); // Display map elements pairing numeric inputs to explicit day terms
					CmdLCD(GOTO_L2_POSN0); // Shift visual target parameters down onto row number two boundaries
					StrLCD((signed char*)"4.T 5.F 6.SA 7.Q"); // Paint remainder lookup codes choices out onto screen cells
					key = KeyScan();  // Scan active keyboard components to extract operator day mapping choice
					if(!(key >= '0' && key <= '7')) // Test inputs to ensure characters remain strictly within option fields
					{
						CmdLCD(CLR);    // Wipe dynamic hardware panel text layers completely from active configurations
						CmdLCD(GOTO_L1_POSN0); // Relocate active text drawing points up onto top boundary zones
						StrLCD((signed char*)"Invalid Day"); // Render warning showing target index maps fall outside definitions
						CmdLCD(GOTO_L2_POSN0); // Drop structural printing bounds down onto row two coordinate fields
						StrLCD((signed char*)"Try Again"); // Render contextual instruction guidelines asking for correct inputs
						delay_ms(100);  // Halt internal application processing paths for basic read windows
					}
					else if(key == '7') // Evaluate if the operator entered the dedicated exit sequence option
					{
						break;          // Terminate standard day processing sub-loops completely via break statements
					}
					else                // Execute data update tasks when keys successfully map within correct scopes
					{
						SetRTCDay((int)key - '0'); // Convert numeric characters to raw integers to commit new days to RTC
						CmdLCD(CLR);    // Wipe displaying lines clean to showcase transaction success screens
						CmdLCD(GOTO_L1_POSN0); // Snap visualization pointers back up to top-left matrix nodes
						StrLCD((signed char*)"Day saved!"); // Display success feedback verifying updates were written to memory
						delay_ms(500);  // Freeze processing tracks across a brief half-second verification interval
					}
				}
				break;              // Drop execution tracks down from active day modification nodes completely
				
			// RTC date
			case '3':               // Branch routing structural execution into calendars date update tracking nodes
				while(1)            // Initialize persistent user blocking loop to capture numeric date component inputs
				{
					CmdLCD(CLR);      // Execute panel clear operations to refresh standard alpha-numeric outputs
					CmdLCD(GOTO_L1_POSN0); // Force layout coordinate focus targets up onto line number one margins
					StrLCD((signed char*)date); // Output active global calendar date formatting strings to screen faces
					CmdLCD(GOTO_L1_POSN0 + 11); // Push line location targets eleven blocks rightward to frame exit labels
					StrLCD((signed char*)"4.exit"); // Draw standardized boundaries escape menu option representations
					CmdLCD(GOTO_L2_POSN0); // Reset character printing focus down onto row level number two markers
					StrLCD((signed char*)"1.dt 2.mon 3.yr"); // Output structural sub-menu options choices down across screen width
					key = KeyScan();  // Interrogate key hardware networks to pull current user decisions parameters
					switch(key)       // Parse active sub-menu inputs to split processing down individual sub-paths
					{
						// date
						case '1':       // Branch path handling individual month day numeric layout alterations
						i: CmdLCD(CLR);  // Clear interface lines to track entry sequences accurately without ghosts
							CmdLCD(GOTO_L1_POSN0); // Re-align structural display focus lines with top line starting zones
							StrLCD((signed char*)date); // Paint the active calendar date tracking string structures to viewports
							CmdLCD(GOTO_L1_POSN0); // Bring cursor alignment straight back onto first day digit offset
							date[0] = KeyScan(); // Block task tracking sequences until keypad returns a first tens digit
							CharLCD(date[0]); // Write dynamic visual confirmation echo characters directly back to user cells
							if(date[0] == 'C') // Evaluate if user intends to fall back and drop inputs
							{
								date[0] = '0'; // Restore baseline safe placeholders down inside memory targets
								goto i;     // Force local flow control paths back to top level loop tracker labels
							}
							else if(date[0] >= '0' && date[0] <= '3') // Check that first date digit fits calendar requirements (0-3)
							{
							date: CmdLCD(CLR); // Refresh system layout visualization windows by flashing matrix clear commands
								CmdLCD(GOTO_L1_POSN0); // Return printing boundaries targets back onto primary starting positions
								StrLCD((signed char*)date); // Draw the composite calendar structural values array string back out
								CmdLCD(GOTO_L1_POSN0 + 1); // Jump text cursor parameters forward to match unit day indices fields
								date[1] = KeyScan(); // Capture second individual character numeric digit down into index one
								if(date[1] == 'C') // Test inputs to verify if user triggered system reset adjustments
								{
									date[0] = '0'; // Reset structural component parameters inside local arrays back to base
									date[1] = '0'; // Reset structural component parameters inside local arrays back to base
									goto i; // Return tracking paths backward onto primary date parameter labels lines
								}
								else if((date[0] == '0' && (date[1] >= '1' && date[1] <= '9'))
									|| ((date[0] >= '1' && date[0] <= '2') && (date[1] >= '0' && date[1] <= '9'))
									|| (date[0] == '3' && (date[1] >= '0') && date[1] <= '1')) // Validate standard calendar rules (01-31)
								{
									CharLCD(date[1]); // Render valid target character values directly to LCD coordinates
									SetRTCDateInfo(date); // Push new string configurations straight down to hardware RTC chips
								}
								else        // Catch layout violations where dates exceed month limits definitions
								{
									date[1] = '0'; // Replace inaccurate value components with safe baseline zero indicators
									CmdLCD(CLR); // Request absolute matrix clear passes down to hardware screens controllers
									CmdLCD(GOTO_L1_POSN0); // Focus visualization grids onto top layer alignment nodes
									StrLCD((signed char*)"Invalid Date"); // Write error text notifying operator concerning broken boundaries parameters
									CmdLCD(GOTO_L2_POSN0); // Move graphic context indicators onto baseline secondary rows
									StrLCD((signed char*)"Try Again"); // Print out clear manual restart paths inside option displays
									delay_ms(100); // Wait out standard time intervals to ensure readability before updates
									goto date; // Route internal steps back to localized unit error catch points
								}
							}
							else          // Triggered when initial date input characters break base structural limits
							{
								date[0] = '0'; // Enforce safe structural defaults back into target data cells
								CmdLCD(CLR); // Blank display fields to allow new error strings presentations
								CmdLCD(GOTO_L1_POSN0); // Point alignment cursors back at primary interface starting offsets
								StrLCD((signed char*)"Invalid Date"); // Inform user that date entries fall completely out of bounds
								CmdLCD(GOTO_L2_POSN0); // Move text generation targets down onto lower tracking structures
								StrLCD((signed char*)"Try Again"); // Render contextual interface retry messages inside line grids
								delay_ms(100); // Suspend computing updates across brief diagnostic timing windows
								goto i;     // Route background processing logic straight back up onto label i
							}
							break;        // Terminate individual date day modifications paths cleanly via breaks
						
						// month
						case '2':       // Branch path handling month configurations transformations within systems
						j: CmdLCD(CLR);  // Command active viewing devices to drop current text layout maps
							CmdLCD(GOTO_L1_POSN0); // Relocate baseline writing targets up onto row number one origins
							StrLCD((signed char*)date); // Draw the current layout template calendar information to panels faces
							CmdLCD(GOTO_L1_POSN0 + 3); // Position the text tracking cursor straight onto tens month index location
							date[3] = KeyScan(); // Read matrix button inputs directly into data index string element three
							CharLCD(date[3]); // Present updated numeric selections directly onto screen grid lines
							if(date[3] == 'C') // Check for intentional clear overrides entered through keyboard systems
							{
								date[3] = '0'; // Clear out variable indexes back into baseline structural default values
								goto i;     // Jump background execution tracks far backward back to label entry i
							}
							else if(date[3] >= '0' && date[3] <= '1') // Ensure month tens character matches correct parameters (0-1)
							{
							month: CmdLCD(CLR); // Refresh interface screen frames by running matrix sweep routines
								CmdLCD(GOTO_L1_POSN0); // Reset text generation cursors back onto top boundary layers
								StrLCD((signed char*)date); // Print operational calendar arrays onto character fields grids
								CmdLCD(GOTO_L1_POSN0 + 4); // Jump cursor points forward to match unit month textual positions
								date[4] = KeyScan(); // Extract unit month selections parameters directly into index four cells
								if(date[4] == 'C') // Test user choices to evaluate if a clear override was initiated
								{
									date[3] = '0'; // Force active tracking indexes back down to default zero strings
									date[4] = '0'; // Force active tracking indexes back down to default zero strings
									goto j; // Loop structural systems flow back up to internal label references j
								}
								else if((date[3] == '0' && (date[4] >= '1' && date[4] <= '9')) ||
									(date[3] == '1' && (date[4] >= '0' && date[4] <= '2'))) // Validate month values (01-12)
								{
									CharLCD(date[1]); // Render dynamic symbol elements directly into current layout blocks
									SetRTCDateInfo(date); // Save valid date configurations into physical background hardware chips
								}
								else        // Execute fallback operations if the composite month parameters break bounds
								{
									date[4] = '0'; // Overwrite invalid components back into clean structural placeholders
									CmdLCD(CLR); // Clear display rows before presenting explicit boundary error readouts
									CmdLCD(GOTO_L1_POSN0); // Relocate dynamic focus targets back onto top row fields line
									StrLCD((signed char*)"Invalid month"); // Output clear diagnostic notifications describing boundary failures
									CmdLCD(GOTO_L2_POSN0); // Step text cursors down onto lower utility row zones
									StrLCD((signed char*)"Try Again"); // Frame basic re-entry instructions down inside layout boxes
									delay_ms(100); // Suspend foreground compute routines across brief delay periods
									goto month; // Transfer execution pathways back down to localized sub-menus labels
								}
							}
							else          // Triggered when initial month selections violate design definitions completely
							{
								date[3] = '0'; // Revert un-validated parameter variables back into safe string conditions
								CmdLCD(CLR); // Throw general matrix wipe codes out through communications interfaces
								CmdLCD(GOTO_L1_POSN0); // Re-align printing pointers back up with top line starting grids
								StrLCD((signed char*)"Invalid month"); // Report range failures back down across liquid crystal screen tracks
								CmdLCD(GOTO_L2_POSN0); // Drop focus targets down onto baseline secondary screen layers
								StrLCD((signed char*)"Try Again"); // Re-render interactive instructions urging operational retries
								delay_ms(100); // Stall processing paths to ensure error strings are fully legible
								goto j;     // Jump system flow tracks back up to structural entry point j
							}
							break;        // Drop out from active month adjustments sub-cases completely via breaks
						
						// year
						case '3':       // Branch node handling numerical 4-digit calendar year value modifications
						k: CmdLCD(CLR);  // Clear dynamic panel viewing fields to configure clean data entry modes
							StrLCD((signed char*)"Year(0-4095):"); // Output textual constraint guidelines showing allowable ranges
							CmdLCD(GOTO_L2_POSN0); // Move layout drawing points down onto secondary row boundaries lines

							// Collect 4 digits and check each one for "numeric only"
							for(i = 0; i < 4; i++) // Initialize iterative loops to sequentially capture four numbers
							{
								temp = KeyScan(); // Poll keypad peripheral arrays to secure individual characters inputs
								
								// 1. Check if the key is a digit (0-9)
								if(temp >= '0' && temp <= '9') // Verify inputs match valid mathematical numeric parameters
								{
									date[6 + i] = temp; // Store verified numeric items into correct target offset arrays
									CharLCD(temp);   // Write character symbols down to visual liquid crystal display cells
								}
								// 2. If 'C' is pressed, restart
								else if(temp == 'C') // Check if users requested to dump entries and fall back
								{
									goto k;         // Loop functional execution paths directly back to label entry k
								}
								// 3. If anything else is pressed (like +, A, B, #)
								else                // Catch node treating illegal symbols selections during numeric inputs
								{
									CmdLCD(CLR);    // Clear interface strings immediately from display panels memory maps
									StrLCD((signed char*)"Invalid Key!"); // Render quick notifications verifying inputs matched unmapped keys
									delay_ms(1000); // Stand down computing tracks for a duration spanning one second
									goto k;         // Force execution routines to return back to loop label k
								}
							}

							// --- After the loop, we have exactly 4 digits ---
							year_val = ((date[6] - '0') * 1000) + 
							           ((date[7] - '0') * 100) + 
							           ((date[8] - '0') * 10) + 
							           (date[9] - '0'); // Reconstitute raw characters arrays elements into one composite integer value

							if(year_val > 4095) // Validate composite integer totals fall inside hardware constraints (Max 4095)
							{
								CmdLCD(CLR);      // Request absolute visual panels clear actions from local modules
								StrLCD((signed char*)"Max is 4095!"); // Render textual notification warning of max threshold limits
								delay_ms(1000);   // Enforce strict one-second wait periods to capture user eye focuses
								goto k;           // Force loop flow backwards up to localized year initialization blocks
							}
							else                // Path chosen when the constructed year integer satisfies design guidelines
							{
								SetRTCDateInfo(date); // Save fully validated calendar configurations to background clock systems
								StrLCD((signed char*)" - Saved!"); // Render short validation statements showing task success flags
								delay_ms(1000);   // Pause foreground loops for one second to visually display confirmation
							}
							break;        // Break away from year calculation routines back into main structural layers
						case '4':       // Sub-menu terminal option choice indicating user configuration escape paths
							break;        // Drop out from target menu layers switch statements cleanly via breaks
						default:        // Trapping block parsing entries that fail to match standard numbers codes
							CmdLCD(CLR);  // Command active visual panels to empty contents from display matrices
							CmdLCD(GOTO_L1_POSN0); // Move graphic focus indicators up onto start row coordinates line
							StrLCD((signed char*)"Invalid Input"); // Output general error texts highlighting unexpected system arguments
							CmdLCD(GOTO_L2_POSN0); // Pull layout writing focuses down onto lower boundary lines tracks
							StrLCD((signed char*)"Try Again"); // Display instructional terms inviting users to input choices again
							delay_ms(1000); // Block operational updates across a span tracking full single seconds
							break;        // Break cleanly out from calendar selections blocks back to parent routines
					}
					if(key == '4')    // Explicit validation step checking if escape codes are currently flagged
						break;          // Disregard sub-loops limits and jump straight out of nested structures
				}
				break;              // Escape execution pathways down through outer calendar processing arrays
						
			case '4':               // Standard exit key match terminating settings changes routines entirely
				break;              // Break cleanly away from baseline secondary menu structures via breaks
			default:                // Fallback catch parsing options selections falling outside primary boundaries
				CmdLCD(CLR);        // Clear visual panels completely to handle invalid option displays safely
				CmdLCD(GOTO_L1_POSN0); // Shift printing alignments back to upper-left interface markers
				StrLCD((signed char*)"Invalid Input"); // Write error line explaining chosen keys are unmapped options
				CmdLCD(GOTO_L2_POSN0); // Drop focus targets onto bottom display rows to finalize feedback
				StrLCD((signed char*)"Try Again"); // Render simple operational instruction blocks encouraging re-entry
				delay_ms(1000);     // Hold messaging active across a duration window lasting one full second
				break;              // Break away from parent operational menu processing statements blocks
		}
		if(key == '4')              // Explicit structural check tracking system exit conditions parameters inside states
			break;                  // Terminate master secondary interface configurations tracking loops completely
	}
}

void configure(void)            // Implement primary system setup routine managing overall reminders configurations parameters
{
	flag = 0;                     // Reset system activation request flags back down to operational baseline zero
	while(1)                      // Establish infinite master tracking loop handling primary settings selection screens
	{
		/*menu-1
		1.time,date,day editing
		2.New Entry or update medicine details and Remainders
		3.exit menu and display real time clock*/
		CmdLCD(CLR);                // Command LCD screen controllers to clear dynamic buffers from display lines
		CmdLCD(GOTO_L1_POSN0);      // Lock layout focus positions back onto origin elements of first text lines
		StrLCD((signed char*)"1.RTC Edit 3.Q"); // Write out options labels for master level selections paths layout
		CmdLCD(GOTO_L2_POSN0);      // Drop dynamic cursor focus alignments onto secondary terminal display lines
		StrLCD((signed char*)"2.Medicine Edit"); // Draw secondary structural text parameters across the screen layout width
		key = KeyScan();            // Poll physical keypad networks to catch initial navigation commands choice
		// Based on key value run the process
		
		switch(key)                 // Parse inputs values to direct internal processing tracks down user channels
		{
			/*menu 2
			1.hour,2.min,3.sec,4.day,5.date,6.month,7.year,8.quit*/
			case '1':                 // Option selector branch linking users directly to real clock tracking changes
				menu2();                // Invoke system secondary configurations sub-menus to adjust RTC metrics
				break;                  // Drop system flow straight out from active selections switch matrices
			
			/*menu 3
			1.1st medicine details 2.2nd medicine details 3.3rd medicine details*/
			case '2':                 // Option selector branch routing users to updates medicine alert definitions
				while(1)              // Initialize looping structures to choose which reminder indices to update
				{
					CmdLCD(CLR);        // Clear data characters completely from text rows before printing fresh choices
					CmdLCD(GOTO_L1_POSN0); // Bring visual display pointer targets back up onto line number one margins
					StrLCD((signed char*)"1.Med1 3.Med3"); // Print labels linking specific choices keys to explicit medicine tags
					CmdLCD(GOTO_L2_POSN0); // Step layout creation focus indices down onto lower displaying sectors
					StrLCD((signed char*)"2.Med2 4.Exit"); // Render secondary blocks options allowing quick systems menu exits
					key = KeyScan();    // Scan peripheral keyboard arrays to pull active reminder target items
					switch(key)         // Navigate internal flow channels based on targeted medicine records chosen
					{
						// for medicine 1
						case '1':         // Branch routine configuring runtime hours and minutes for Reminder Block 1
							while(1)      // Initialize persistent configurations adjustment sub-loops for Medicine 1
							{
								CmdLCD(CLR); // Request active displaying panels to strip text parameters off grids
								CmdLCD(GOTO_L1_POSN0); // Target row alignment focus back onto top boundary start locations
								StrLCD((signed char*)m1); // Render active text strings tracking settings for Reminder 1
								CmdLCD(GOTO_L2_POSN0); // Shift text generation targets onto lower tracking structure layers
								StrLCD((signed char*)"1.hr 2.min 3.Q"); // Present specific component adjustment options items back to user
								key = KeyScan(); // Catch target matrix inputs data characters from local systems
								switch(key) // Parse component inputs flags to split processing tracks down choices paths
								{
									// for hour set
									case '1': // Target sub-branch assigned to configure reminder hour variables
									a: CmdLCD(CLR); // Wipe active liquid crystal rendering screens completely from visibility
										CmdLCD(GOTO_L1_POSN0); // Snap visualization pointers straight back onto row one base indices
										StrLCD((signed char*)m1); // Render active tracking templates containing reminder string parameters
										CmdLCD(GOTO_L1_POSN0); // Re-establish text processing locations back up on line entry thresholds
										m1[0] = KeyScan(); // Block for single-character digit key matrix selection to insert at offset 0
										CharLCD(m1[0]); // Draw input characters directly down across hardware visual nodes
										if(m1[0] == 'C') // Verify if the operator tapped the dedicated clear override character
										{
											m1[0] = '0'; // Revert top layer cell index mapping back to clean base character
											goto a; // Loop workflow operations down into local fallback timing labels
										}
										else if(m1[0] >= '0' && m1[0] <= '2') // Validate that hours first numeric index fits time limits (0-2)
										{
										m1h: CmdLCD(CLR); // Clean visual interface displays by pushing clear messages out
											CmdLCD(GOTO_L1_POSN0); // Recenter text printing bounds back onto main row paths boundary
											StrLCD((signed char*)m1); // Render layout formatting templates matching ongoing adjustments strings
											CmdLCD(GOTO_L1_POSN0 + 1); // Step text processing locations directly onto unit hours index slots
											m1[1] = KeyScan(); // Extract unit hours selection parameters directly into index one cells
											if(m1[1] == 'C') // Test user choices to evaluate if a clear override was initiated
											{
												m1[0] = '0'; // Overwrite invalid components back into clean structural placeholders
												m1[1] = '0'; // Overwrite invalid components back into clean structural placeholders
												goto a; // Branch control execution tracks back to the primary hours label baseline
											}
											else if(((m1[0] >= '0' && m1[0] <= '1') && (m1[1] >= '0' && m1[1] <= '9'))
												|| (m1[0] == '2' && (m1[1] >= '0') && (m1[1] <= '3'))) // Check for valid hours boundaries (00-23)
											{
												CharLCD(m1[1]); // Render valid chosen symbol entry directly into display view line locations
											}
											else  // Handle error responses if the inputted digit fails numerical ranges
											{
												m1[1] = '0'; // Enforce safe baseline default symbols down inside memory matrices
												CmdLCD(CLR); // Direct display hardware modules to drop current layout structures
												CmdLCD(GOTO_L1_POSN0); // Move graphic rendering references back up onto baseline boundaries
												StrLCD((signed char*)"Invalid Hour"); // Output contextual notifications describing execution constraint issues
												CmdLCD(GOTO_L2_POSN0); // Step cursor markers down onto lower text track layers
												StrLCD((signed char*)"Try Again"); // Prompt interface re-tries by placing system display strings
												delay_ms(100); // Stall standard computing tracks across short millisecond gaps
												goto m1h; // Loop system execution pointers backward back up onto primary element label entry
											}
										}
										else    // Fallback tracking block triggered when the initial hour character fails tests
										{
											m1[0] = '0'; // Revert top layer cell index mapping back to clean base character
											CmdLCD(CLR); // Clear active viewing screen cells to handle error updates cleanly
											CmdLCD(GOTO_L1_POSN0); // Re-establish graphic printing bounds parameters at origin marker points
											StrLCD((signed char*)"Invalid Hour"); // Display notice stating primary data choices failed validation bounds
											CmdLCD(GOTO_L2_POSN0); // Drop graphic cursor focus paths down to next character track lines
											StrLCD((signed char*)"Try Again"); // Present operational retry text blocks back to user view spaces
											delay_ms(100); // Stall dynamic calculation execution tracks briefly for sensory visibility
											goto a; // Branch control flow tracks directly back to target marker label a
										}	
										break;  // Break outward from hour case routine paths back into main tracking structures

									// for min set
									case '2': // Target sub-branch assigned to configure reminder minute variables
									b: CmdLCD(CLR); // Reset display screen elements by pushing a clear message down to registers
										CmdLCD(GOTO_L1_POSN0); // Establish target display field alignment coordinates back to Row 1 start
										StrLCD((signed char*)m1); // Display reminder variable data structures across the character pane surface
										CmdLCD(GOTO_L1_POSN0 + 3); // Advance focus alignment forward across the text array onto tens minute index
										m1[3] = KeyScan(); // Capture target matrix index value selection to record at offset three
										CharLCD(m1[3]); // Display standard numeric symbol mapping character values down to LCD grids
										if(m1[3] == 'C') // Test array strings to determine if an abort command request was given
										{
											m1[3] = '0'; // Revert target element memory register states back to base character zero
											goto b; // Re-route processing logic flow tracks backward onto loop entry label b
										}
										else if(m1[3] >= '0' && m1[3] <= '5') // Verify the tens character fits standard hexagesimal scales (0-5)
										{
										m1m: CmdLCD(CLR); // Clean the active system displaying elements framework back to base states
											CmdLCD(GOTO_L1_POSN0); // Relocate textual view cursor alignment locations back to the row origin
											StrLCD((signed char*)m1); // Reprint the underlying reminder layout contents to dynamic panel fields
											CmdLCD(GOTO_L1_POSN0 + 4); // Step cursor forward four blocks to target structural unit minute position
											m1[4] = KeyScan(); // Read explicit key element assignment data mapping directly into offset four
											if(m1[4] == 'C') // Evaluate if user intends to fall back and purge local parameters
											{
												m1[3] = '0'; // Restore context column tracking index data values back to baseline
												m1[4] = '0'; // Clear out unit slot structure fields back to structural baseline
												goto b; // Re-align loop pointers back to primary branch parameter label markers
											}
											else if(m1[4] >= '0' && m1[4] <= '9') // Confirm that unit digit inputs reside cleanly inside numerical values
											{
												CharLCD(m1[4]); // Draw single character unit digit onto display structure array blocks
											}
											else  // Handle error responses if the inputted digit fails numerical ranges
											{
												m1[4] = '0'; // Enforce fallback data definitions down onto active parameters cells
												CmdLCD(CLR); // Refresh displaying interfaces by throwing structural wipe controls down
												CmdLCD(GOTO_L1_POSN0); // Re-establish text processing locations back up on line entry thresholds
												StrLCD((signed char*)"Invalid Minute"); // Render confirmation alert string details indicating boundary error
												CmdLCD(GOTO_L2_POSN0); // Move graphic focus indicators down onto secondary row frameworks
												StrLCD((signed char*)"Try Again"); // Encourage interactive user retries by populating layout prompts
												delay_ms(100); // Block operation steps briefly to make data updates visual to eyes
												goto m1m; // Loop workflow operations down into local fallback minute logic paths
											}
										}
										else    // Triggered when initial minute values exceed maximum timing definitions
										{
											m1[3] = '0'; // Set index memory location contents straight back to reliable zero base
											CmdLCD(CLR); // Flash clear command updates across character displaying module blocks
											CmdLCD(GOTO_L1_POSN0); // Align tracking focus limits back up with main line text fields
											StrLCD((signed char*)"Invalid Minute"); // Alert operators concerning incorrect minutes input structural constraints
											CmdLCD(GOTO_L2_POSN0); // Drop writing coordinates down onto secondary interface layer regions
											StrLCD((signed char*)"Try Again"); // Present structural user retry options down onto layout grids
											delay_ms(100); // Hold operational flow tracking suspended across short tracking gaps
											goto b; // Loop operational configurations direct to step marker label reference b
										}
										break;  // Break cleanly out from minute handling cases back into parent structures
									
									case '3': // User request option mapping directly to dynamic menu exits paths
										break;  // Break away from individual component modifications switch structures cleanly
									
									default:  // Trapping case capturing unassigned keystroke signals safely
										CmdLCD(CLR); // Request standard display updates to reset visual grids paths
										CmdLCD(GOTO_L1_POSN0); // Reset textual view coordinates back up onto primary margins
										StrLCD((signed char*)"Invalid Input"); // Output context error texts validating selection bounds failures
										CmdLCD(GOTO_L2_POSN0); // Push layout rendering target paths down onto row level two
										StrLCD((signed char*)"Try Again"); // Display manual instruction strings prompting layout choices re-entry
										delay_ms(1000); // Freeze system configurations workflows across full single seconds windows
										break;  // Drop out from current reminder adjustments sub-cases completely
								}
								if(key == '3') // Check if termination request metrics match active key variables
									break;      // Escape persistent configuration management tracking loops completely
							}
							break;        // Break outward from specific Reminder 1 adjustments paths entirely

						// for medicine 2
						case '2':         // Branch routine configuring runtime hours and minutes for Reminder Block 2
							while(1)      // Initialize persistent configurations adjustment sub-loops for Medicine 2
							{
								CmdLCD(CLR); // Request active displaying panels to strip text parameters off grids
								CmdLCD(GOTO_L1_POSN0); // Target row alignment focus back onto top boundary start locations
								StrLCD((signed char*)m2); // Render active text strings tracking settings for Reminder 2
								CmdLCD(GOTO_L2_POSN0); // Shift text generation targets onto lower tracking structure layers
								StrLCD((signed char*)"1.hr 2.min 3.Q"); // Present specific component adjustment options items back to user
								key = KeyScan(); // Catch target matrix inputs data characters from local systems
								switch(key) // Parse component inputs flags to split processing tracks down choices paths
								{
									// for hour set
									case '1': // Target sub-branch assigned to configure reminder hour variables
									c: CmdLCD(CLR); // Wipe active liquid crystal rendering screens completely from visibility
										CmdLCD(GOTO_L1_POSN0); // Snap visualization pointers straight back onto row one base indices
										StrLCD((signed char*)m2); // Render active tracking templates containing reminder string parameters
										CmdLCD(GOTO_L1_POSN0); // Re-establish text processing locations back up on line entry thresholds
										m2[0] = KeyScan(); // Block for single-character digit key matrix selection to insert at offset 0
										CharLCD(m2[0]); // Draw input characters directly down across hardware visual nodes
										if(m2[0] == 'C') // Verify if the operator tapped the dedicated clear override character
										{
											m2[0] = '0'; // Revert top layer cell index mapping back to clean base character
											goto c; // Loop workflow operations down into local fallback timing labels
										}
										else if(m2[0] >= '0' && m2[0] <= '2') // Validate that hours first numeric index fits time limits (0-2)
										{
										m2h: CmdLCD(CLR); // Clean visual interface displays by pushing clear messages out
											CmdLCD(GOTO_L1_POSN0); // Recenter text printing bounds back onto main row paths boundary
											StrLCD((signed char*)m2); // Render layout formatting templates matching ongoing adjustments strings
											CmdLCD(GOTO_L1_POSN0 + 1); // Step text processing locations directly onto unit hours index slots
											m2[1] = KeyScan(); // Extract unit hours selection parameters directly into index one cells
											if(m2[1] == 'C') // Test user choices to evaluate if a clear override was initiated
											{
												m2[0] = '0'; // Overwrite invalid components back into clean structural placeholders
												m2[1] = '0'; // Overwrite invalid components back into clean structural placeholders
												goto c; // Branch control execution tracks back to the primary hours label baseline
											}
											else if(((m2[0] >= '0' && m2[0] <= '1') && (m2[1] >= '0' && m2[1] <= '9'))
												|| (m2[0] == '2' && (m2[1] >= '0') && (m2[1] <= '3'))) // Check for valid hours boundaries (00-23)
											{
												CharLCD(m2[1]); // Render valid chosen symbol entry directly into display view line locations
											}
											else  // Handle error responses if the inputted digit fails numerical ranges
											{
												m2[1] = '0'; // Enforce safe baseline default symbols down inside memory matrices
												CmdLCD(CLR); // Direct display hardware modules to drop current layout structures
												CmdLCD(GOTO_L1_POSN0); // Move graphic rendering references back up onto baseline boundaries
												StrLCD((signed char*)"Invalid Hour"); // Output contextual notifications describing execution constraint issues
												CmdLCD(GOTO_L2_POSN0); // Step cursor markers down onto lower text track layers
												StrLCD((signed char*)"Try Again"); // Prompt interface re-tries by placing system display strings
												delay_ms(100); // Stall standard computing tracks across short millisecond gaps
												goto m2h; // Loop system execution pointers backward back up onto primary element label entry
											}
										}
										else    // Fallback tracking block triggered when the initial hour character fails tests
										{
											m2[0] = '0'; // Revert top layer cell index mapping back to clean base character
											CmdLCD(CLR); // Clear active viewing screen cells to handle error updates cleanly
											CmdLCD(GOTO_L1_POSN0); // Re-establish graphic printing bounds parameters at origin marker points
											StrLCD((signed char*)"Invalid Hour"); // Display notice stating primary data choices failed validation bounds
											CmdLCD(GOTO_L2_POSN0); // Drop graphic cursor focus paths down to next character track lines
											StrLCD((signed char*)"Try Again"); // Present operational retry text blocks back to user view spaces
											delay_ms(100); // Stall dynamic calculation execution tracks briefly for sensory visibility
											goto c; // Branch control flow tracks directly back to target marker label c
										}	
										break;  // Break outward from hour case routine paths back into main tracking structures

									// for min set
									case '2': // Target sub-branch assigned to configure reminder minute variables
									d: CmdLCD(CLR); // Reset display screen elements by pushing a clear message down to registers
										CmdLCD(GOTO_L1_POSN0); // Establish target display field alignment coordinates back to Row 1 start
										StrLCD((signed char*)m2); // Display reminder variable data structures across the character pane surface
										CmdLCD(GOTO_L1_POSN0 + 3); // Advance focus alignment forward across the text array onto tens minute index
										m2[3] = KeyScan(); // Capture target matrix index value selection to record at offset three
										CharLCD(m2[3]); // Display standard numeric symbol mapping character values down to LCD grids
										if(m2[3] == 'C') // Test array strings to determine if an abort command request was given
										{
											m2[3] = '0'; // Revert target element memory register states back to base character zero
											goto d; // Re-route processing logic flow tracks backward onto loop entry label d
										}
										else if(m2[3] >= '0' && m2[3] <= '5') // Verify the tens character fits standard hexagesimal scales (0-5)
										{
										m2m: CmdLCD(CLR); // Clean the active system displaying elements framework back to base states
											CmdLCD(GOTO_L1_POSN0); // Relocate textual view cursor alignment locations back to the row origin
											StrLCD((signed char*)m2); // Reprint the underlying reminder layout contents to dynamic panel fields
											CmdLCD(GOTO_L1_POSN0 + 4); // Step cursor forward four blocks to target structural unit minute position
											m2[4] = KeyScan(); // Read explicit key element assignment data mapping directly into offset four
											if(m2[4] == 'C') // Evaluate if user intends to fall back and purge local parameters
											{
												m2[3] = '0'; // Restore context column tracking index data values back to baseline
												m2[4] = '0'; // Clear out unit slot structure fields back to structural baseline
												goto d; // Re-align loop pointers back to primary branch parameter label markers
											}
											else if(m2[4] >= '0' && m2[4] <= '9') // Confirm that unit digit inputs reside cleanly inside numerical values
											{
												CharLCD(m2[4]); // Draw single character unit digit onto display structure array blocks
											}
											else  // Handle error responses if the inputted digit fails numerical ranges
											{
												m2[4] = '0'; // Enforce fallback data definitions down onto active parameters cells
												CmdLCD(CLR); // Refresh displaying interfaces by throwing structural wipe controls down
												CmdLCD(GOTO_L1_POSN0); // Re-establish text processing locations back up on line entry thresholds
												StrLCD((signed char*)"Invalid Minute"); // Render confirmation alert string details indicating boundary error
												CmdLCD(GOTO_L2_POSN0); // Move graphic focus indicators down onto secondary row frameworks
												StrLCD((signed char*)"Try Again"); // Encourage interactive user retries by populating layout prompts
												delay_ms(100); // Block operation steps briefly to make data updates visual to eyes
												goto m2m; // Loop workflow operations down into local fallback minute logic paths
											}
										}
										else    // Triggered when initial minute values exceed maximum timing definitions
										{
											m2[3] = '0'; // Set index memory location contents straight back to reliable zero base
											CmdLCD(CLR); // Flash clear command updates across character displaying module blocks
											CmdLCD(GOTO_L1_POSN0); // Align tracking focus limits back up with main line text fields
											StrLCD((signed char*)"Invalid Minute"); // Alert operators concerning incorrect minutes input structural constraints
											CmdLCD(GOTO_L2_POSN0); // Drop writing coordinates down onto secondary interface layer regions
											StrLCD((signed char*)"Try Again"); // Present structural user retry options down onto layout grids
											delay_ms(100); // Hold operational flow tracking suspended across short tracking gaps
											goto d; // Loop operational configurations direct to step marker label reference d
										}
										break;  // Break cleanly out from minute handling cases back into parent structures
									
									case '3': // User request option mapping directly to dynamic menu exits paths
										break;  // Break away from individual component modifications switch structures cleanly
									
									default:  // Trapping case capturing unassigned keystroke signals safely
										CmdLCD(CLR); // Request standard display updates to reset visual grids paths
										CmdLCD(GOTO_L1_POSN0); // Reset textual view coordinates back up onto primary margins
										StrLCD((signed char*)"Invalid Input"); // Output context error texts validating selection bounds failures
										CmdLCD(GOTO_L2_POSN0); // Push layout rendering target paths down onto row level two
										StrLCD((signed char*)"Try Again"); // Display manual instruction strings prompting layout choices re-entry
										delay_ms(1000); // Freeze system configurations workflows across full single seconds windows
										break;  // Drop out from current reminder adjustments sub-cases completely
								}
								if(key == '3') // Check if termination request metrics match active key variables
									break;      // Escape persistent configuration management tracking loops completely
							}
							break;        // Break outward from specific Reminder 2 adjustments paths entirely

						// for medicine 3
						case '3':         // Branch routine configuring runtime hours and minutes for Reminder Block 3
							while(1)      // Initialize persistent configurations adjustment sub-loops for Medicine 3
							{
								CmdLCD(CLR); // Request active displaying panels to strip text parameters off grids
								CmdLCD(GOTO_L1_POSN0); // Target row alignment focus back onto top boundary start locations
								StrLCD((signed char*)m3); // Render active text strings tracking settings for Reminder 3
								CmdLCD(GOTO_L2_POSN0); // Shift text generation targets onto lower tracking structure layers
								StrLCD((signed char*)"1.hr 2.min 3.Q"); // Present specific component adjustment options items back to user
								key = KeyScan(); // Catch target matrix inputs data characters from local systems
								switch(key) // Parse component inputs flags to split processing tracks down choices paths
								{
									// for hour set
									case '1': // Target sub-branch assigned to configure reminder hour variables
									e: CmdLCD(CLR); // Wipe active liquid crystal rendering screens completely from visibility
										CmdLCD(GOTO_L1_POSN0); // Snap visualization pointers straight back onto row one base indices
										StrLCD((signed char*)m3); // Render active tracking templates containing reminder string parameters
										CmdLCD(GOTO_L1_POSN0); // Re-establish text processing locations back up on line entry thresholds
										m3[0] = KeyScan(); // Block for single-character digit key matrix selection to insert at offset 0
										CharLCD(m3[0]); // Draw input characters directly down across hardware visual nodes
										if(m3[0] == 'C') // Verify if the operator tapped the dedicated clear override character
										{
											m3[0] = '0'; // Revert top layer cell index mapping back to clean base character
											goto e; // Loop workflow operations down into local fallback timing labels
										}
										else if(m3[0] >= '0' && m3[0] <= '2') // Validate that hours first numeric index fits time limits (0-2)
										{
										m3h: CmdLCD(CLR); // Clean visual interface displays by pushing clear messages out
											CmdLCD(GOTO_L1_POSN0); // Recenter text printing bounds back onto main row paths boundary
											StrLCD((signed char*)m3); // Render layout formatting templates matching ongoing adjustments strings
											CmdLCD(GOTO_L1_POSN0 + 1); // Step text processing locations directly onto unit hours index slots
											m3[1] = KeyScan(); // Extract unit hours selection parameters directly into index one cells
											if(m3[1] == 'C') // Test user choices to evaluate if a clear override was initiated
											{
												m3[0] = '0'; // Overwrite invalid components back into clean structural placeholders
												m3[1] = '0'; // Overwrite invalid components back into clean structural placeholders
												goto e; // Branch control execution tracks back to the primary hours label baseline
											}
											else if(((m3[0] >= '0' && m3[0] <= '1') && (m3[1] >= '0' && m3[1] <= '9'))
												|| (m3[0] == '2' && (m3[1] >= '0') && (m3[1] <= '3'))) // Check for valid hours boundaries (00-23)
											{
												CharLCD(m3[1]); // Render valid chosen symbol entry directly into display view line locations
											}
											else  // Handle error responses if the inputted digit fails numerical ranges
											{
												m3[1] = '0'; // Enforce safe baseline default symbols down inside memory matrices
												CmdLCD(CLR); // Direct display hardware modules to drop current layout structures
												CmdLCD(GOTO_L1_POSN0); // Move graphic rendering references back up onto baseline boundaries
												StrLCD((signed char*)"Invalid Hour"); // Output contextual notifications describing execution constraint issues
												CmdLCD(GOTO_L2_POSN0); // Step cursor markers down onto lower text track layers
												StrLCD((signed char*)"Try again"); // Prompt interface re-tries by placing system display strings
												delay_ms(100); // Stall standard computing tracks across short millisecond gaps
												goto m3h; // Loop system execution pointers backward back up onto primary element label entry
											}
										}
										else    // Fallback tracking block triggered when the initial hour character fails tests
										{
											m3[0] = '0'; // Revert top layer cell index mapping back to clean base character
											CmdLCD(CLR); // Clear active viewing screen cells to handle error updates cleanly
											CmdLCD(GOTO_L1_POSN0); // Re-establish graphic printing bounds parameters at origin marker points
											StrLCD((signed char*)"Invalid Hour"); // Display notice stating primary data choices failed validation bounds
											CmdLCD(GOTO_L2_POSN0); // Drop graphic cursor focus paths down to next character track lines
											StrLCD((signed char*)"Try again"); // Present operational retry text blocks back to user view spaces
											delay_ms(100); // Stall dynamic calculation execution tracks briefly for sensory visibility
											goto e; // Branch control flow tracks directly back to target marker label e
										}	
										break;  // Break outward from hour case routine paths back into main tracking structures

									// for min set
									case '2': // Target sub-branch assigned to configure reminder minute variables
									f: CmdLCD(CLR); // Reset display screen elements by pushing a clear message down to registers
										CmdLCD(GOTO_L1_POSN0); // Establish target display field alignment coordinates back to Row 1 start
										StrLCD((signed char*)m3); // Display reminder variable data structures across the character pane surface
										CmdLCD(GOTO_L1_POSN0 + 3); // Advance focus alignment forward across the text array onto tens minute index
										m3[3] = KeyScan(); // Capture target matrix index value selection to record at offset three
										CharLCD(m3[3]); // Display standard numeric symbol mapping character values down to LCD grids
										if(m3[3] == 'C') // Test array strings to determine if an abort command request was given
										{
											m3[3] = '0'; // Revert target element memory register states back to base character zero
											goto f; // Re-route processing logic flow tracks backward onto loop entry label f
										}
										else if(m3[3] >= '0' && m3[3] <= '5') // Verify the tens character fits standard hexagesimal scales (0-5)
										{
										m3m: CmdLCD(CLR); // Clean the active system displaying elements framework back to base states
											CmdLCD(GOTO_L1_POSN0); // Relocate textual view cursor alignment locations back to the row origin
											StrLCD((signed char*)m3); // Reprint the underlying reminder layout contents to dynamic panel fields
											CmdLCD(GOTO_L1_POSN0 + 4); // Step cursor forward four blocks to target structural unit minute position
											m3[4] = KeyScan(); // Read explicit key element assignment data mapping directly into offset four
											if(m3[4] == 'C') // Evaluate if user intends to fall back and purge local parameters
											{
												m3[3] = '0'; // Restore context column tracking index data values back to baseline
												m3[4] = '0'; // Clear out unit slot structure fields back to structural baseline
												goto f; // Re-align loop pointers back to primary branch parameter label markers
											}
											else if(m3[4] >= '0' && m3[4] <= '9') // Confirm that unit digit inputs reside cleanly inside numerical values
											{
												CharLCD(m3[4]); // Draw single character unit digit onto display structure array blocks
											}
											else  // Handle error responses if the inputted digit fails numerical ranges
											{
												m3[4] = '0'; // Enforce fallback data definitions down onto active parameters cells
												CmdLCD(CLR); // Refresh displaying interfaces by throwing structural wipe controls down
												CmdLCD(GOTO_L1_POSN0); // Re-establish text processing locations back up on line entry thresholds
												StrLCD((signed char*)"Invalid Minute"); // Render confirmation alert string details indicating boundary error
												CmdLCD(GOTO_L2_POSN0); // Move graphic focus indicators down onto secondary row frameworks
												StrLCD((signed char*)"Try again"); // Encourage interactive user retries by populating layout prompts
												delay_ms(100); // Block operation steps briefly to make data updates visual to eyes
												goto m3m; // Loop workflow operations down into local fallback minute logic paths
											}
										}
										else    // Triggered when initial minute values exceed maximum timing definitions
										{
											m3[3] = '0'; // Set index memory location contents straight back to reliable zero base
											CmdLCD(CLR); // Flash clear command updates across character displaying module blocks
											CmdLCD(GOTO_L1_POSN0); // Align tracking focus limits back up with main line text fields
											StrLCD((signed char*)"Invalid Minute"); // Alert operators concerning incorrect minutes input structural constraints
											CmdLCD(GOTO_L2_POSN0); // Drop writing coordinates down onto secondary interface layer regions
											StrLCD((signed char*)"Try again"); // Present structural user retry options down onto layout grids
											delay_ms(100); // Hold operational flow tracking suspended across short tracking gaps
											goto f; // Loop operational configurations direct to step marker label reference f
										}
										break;  // Break cleanly out from minute handling cases back into parent structures
									
									case '3': // User request option mapping directly to dynamic menu exits paths
										break;  // Break away from individual component modifications switch structures cleanly
									
									default:  // Trapping case capturing unassigned keystroke signals safely
										CmdLCD(CLR); // Request standard display updates to reset visual grids paths
										CmdLCD(GOTO_L1_POSN0); // Reset textual view coordinates back up onto primary margins
										StrLCD((signed char*)"Invalid Input"); // Output context error texts validating selection bounds failures
										CmdLCD(GOTO_L2_POSN0); // Push layout rendering target paths down onto row level two
										StrLCD((signed char*)"Try Again"); // Display manual instruction strings prompting layout choices re-entry
										delay_ms(1000); // Freeze system configurations workflows across full single seconds windows
										break;  // Drop out from current reminder adjustments sub-cases completely
								}
								if(key == '3') // Check if termination request metrics match active key variables
									break;      // Escape persistent configuration management tracking loops completely
							}
							break;        // Break outward from specific Reminder 3 adjustments paths entirely
				
						case '4':         // Dynamic menu selection mapped directly to exit targets within medicine selection states
							break;          // Escape out from the active targeted medicine list selection routine
						default:          // Sub-layer fallback handling unexpected inputs entries safely
							CmdLCD(CLR);    // Flush visual matrix layers to wipe remnants of previous options strings
							CmdLCD(GOTO_L1_POSN0); // Lock text processing alignment pointers back onto upper row bounds
							StrLCD((signed char*)"Invalid Input"); // Write error notices stating selected keys matched no dynamic paths
							CmdLCD(GOTO_L2_POSN0); // Move lower cursor references onto target interface frames rows
							StrLCD((signed char*)"Try Again"); // Frame standard prompt lines inviting system retries parameters
							delay_ms(1000); // Suspend foreground computing across a full single-second timing frame
							break;          // Drop out from current medicine profile select blocks completely
					}
					if(key == '4')      // Check if general system configuration exit commands were issued
						break;            // Drop structural processing loops back to master configurations steps
				}
				break;                  // Terminate active operations down medicine adjustments switch paths entirely
		
			case '3':                   // Global configuration screen escape code index selector mapping
				break;                  // Break away from top-level system choices arrays completely via breaks
		
			default:                    // Baseline catch block processing unhandled inputs keys at global menus states
				CmdLCD(CLR);            // Erase textual elements across display modules prior to updating data fields
				CmdLCD(GOTO_L1_POSN0);  // Shift dynamic printing track lines back up to row one margins origin
				StrLCD((signed char*)"Invalid Input"); // Paint descriptive data strings identifying unexpected character flags
				CmdLCD(GOTO_L2_POSN0);  // Target bottom row text areas before placing basic navigation help labels
				StrLCD((signed char*)"Try Again"); // Write simple layout instruction blocks indicating workflow restart conditions
				delay_ms(1000);         // Stall active operations execution for exactly one full thousand-millisecond block
				break;                  // Break away from primary configurations tracking menu loops entirely
		}
	
		if(key == '3')                // Evaluate condition parameters checking if terminal escape flags were captured
			break;                    // Burst completely out from standard background setup menu systems loops
	}
}