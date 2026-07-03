// ==================================================================================
// FILE: KPM.c
// DESCRIPTION: Matrix Keypad Driver providing initialization, scanning, and decoding
// ==================================================================================

#include<lpc21xx.h>           // Include peripheral register definitions for the NXP LPC21xx Microcontroller.
#include"kpm_defines.h"       // Include hardware pin offset configurations for keypad rows and columns.
#include"KPM.h"               // Include function prototypes and external declarations for keypad scanning.

//unsigned int kpmLUT[4][4]={{1,2,3,4},{5,6,7,8},{9,10,11,12},{13,14,15,16}}; // Commented out alternative 2D array mapping sequential numerical values to matrix indices.
unsigned char kpmLUT[4][4]={{'1','2','3','/'},
									{'4','5','6','*'},
									{'7','8','9','-'},
									{'C','0','=','+'}}; // Define a 4x4 alphanumeric character Look-Up Table aligning layout symbols to button coordinate offsets.

//kpm initialization          // Structural boundary marker indicating the beginning of the keypad initialization block.
void InitKPM(void)            // Define the configuration function to establish basic hardware direction parameters for the matrix.
{                             // Open execution boundary for the keypad peripheral direction configuration routine.
	IODIR1|=15 << ROW0;        // Drive the 4 sequential output row pins to GPIO Output mode by masking four bits from ROW0 offset.
}                             // Close execution block completing physical pin direction adjustments.

//column scan                 // Section label highlighting the immediate start of the asynchronous column status checking function.
int ColScan(void)             // Define a validation function to detect if any connection has been bridged across the input columns.
{                             // Open local execution tracking logic frame for checking input lines state.
	return (((IOPIN1>>COL0)&15)<15)?0:1; // Mask and test the 4 column input bits; returns 0 if any pin is pulled low (key pressed), else 1.
}                             // Terminate function tracking execution block and pass back computed evaluation status integers.

//get row number              // Section boundary notice introducing the localized active row identification tracking technique.
int RowCheck(void)            // Implement a diagnostic algorithm scanning sequential row paths to isolate the precise pressed index.
{                             // Establish stack space frame and open iterative block for the row validation routine.
	int rno;                   // Declare a local index register variable to track loop count iterations and targeted row identifiers.
	for(rno=0;rno<4;rno++)     // Setup a bounded traversal iteration sequence to sequentially cycle through rows 0 to 3.
	{                          // Open context loop boundaries tracking specific sequential row driving instructions.
		IOPIN1=(~(1<<rno))<<ROW0; // Clear exactly one target row pin bit low at a time while preserving the relative pin offset.
		if(ColScan()==0)        // Query column state pins immediately to check if the driven low row line completes a circuit.
		{                      // Enter localized exit path context upon discovering a matched loop criteria condition.
			break;               // Break out of the row traversal tracking loop immediately since the matching row index is captured.
		}                      // Terminate internal conditional boundary checking statements block.
	}                          // Conclude single iteration row drive block and transition to the next loop pass increment.
	// make rows as default    // Text comment notes the operational requirement to re-establish stable idle output states.
	IOPIN1=0x0<<ROW0;          // Restore original hardware default logic state by pulling all row output lines low simultaneously.
	return rno;                // Pass back the final calculated row index integer value to the calling process.
}                             // Exit row check block framework clearing transient computational registers.

//get column number           // Section boundary note introducing the input column pin decoding routine.
int ColCheck(void)            // Define an verification routine to systematically locate which specific column line is currently grounded.
{                             // Open localized context frame block for evaluating specific column pin registers.
	int cno;                   // Declare a tracking loop integer variable to represent the relative column index offset.
	for(cno=0;cno<4;cno++)     // Construct a fixed 4-step loop to traverse and inspect every column coordinate channel.
	{                          // Open loop block context boundary to test a distinct column pin position.
		if((IOPIN1>>(COL0+cno)&1)==0) // Shift target pin status register rightward and check if specific bit reflects a ground zero state.
		{                      // Open boundary frame to handle execution breakout paths upon successful validation.
			break;               // Break from column matching iteration loops immediately as the active column index is isolated.
		}                      // Close specific bit status verification branch tracking blocks.
	}                          // Complete individual tracking index passes and update step counters to examine the adjacent pin.
	return cno;                // Return the successfully isolated active column index number back up to user processes.
}                             // Exit execution block space completing localized columns parsing routines.

char KeyScan(void)            // Define the primary high-level polling function designed to handle user button interface processing.
{                             // Open functional routine tracking block for orchestrating key validation states.
	int rno,cno,keyv;          // Allocate discrete local variable stack frames tracking isolated rows, columns, and absolute decoded symbols.
	//wait for switch press    // Technical comment stating the immediate entry into a blocking keystroke capture state.
	while(ColScan());          // Execute an empty spin-lock loop blocking program flow until any button press pulls a line low.
	//find the row number      // Descriptive message marking transition into row identification parsing phases.
	rno=RowCheck();            // Call row checking module and store the discovered active row index value inside memory tracker.
	// find column number      // Descriptive message marking transition into column identification parsing phases.
	cno=ColCheck();            // Call column checking module and capture the grounded input column path coordinate within local variable.
	//get the key value using kpmLUT // Developer reminder mapping the isolated physical grid coordinates to look-up array data.
	keyv=kpmLUT[rno][cno];     // Extract matching character encoding structure directly out of look-up tables using row and column values.
	//wait for switch release   // Developer note documenting debouncing logic strategy tracking contact break states.
	while(!ColScan());         // Hold code execution patterns frozen within a spin loop until user completely releases the button.
	return keyv;               // Dispatch the extracted alphanumeric character literal directly back up to parent menu routines.
}                             // Terminate core keypad driver scanning loop structure and clear stack footprints.