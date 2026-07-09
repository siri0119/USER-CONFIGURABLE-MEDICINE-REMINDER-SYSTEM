#include<lpc21xx.h>           // Include LPC21xx header file
#include"kpm_defines.h"       // Include keypad pin definitions
#include"KPM.h"               // Include keypad function declarations


// Keypad values
unsigned char kpmLUT[4][4] =
{
	{'1','2','3','/'},
	{'4','5','6','*'},
	{'7','8','9','-'},
	{'C','0','=','+'}
};


// Initialize keypad
void InitKPM(void)
{
	// Set row pins as output
	IODIR1 |= 15 << ROW0;
}


// Check keypad columns
int ColScan(void)
{
	// Return column status
	return (((IOPIN1>>COL0)&15)<15)?0:1;
}


// Find pressed row
int RowCheck(void)
{
	int rno;   // Row number

	// Check all rows
	for(rno=0;rno<4;rno++)
	{
		// Activate one row
		IOPIN1=(~(1<<rno))<<ROW0;

		// Check key press
		if(ColScan()==0)
		{
			// Stop row checking
			break;
		}
	}

	// Set rows to default
	IOPIN1=0x0<<ROW0;

	// Return row number
	return rno;
}


// Find pressed column
int ColCheck(void)
{
	int cno;   // Column number

	// Check all columns
	for(cno=0;cno<4;cno++)
	{
		// Check column pin
		if((IOPIN1>>(COL0+cno)&1)==0)
		{
			// Stop column checking
			break;
		}
	}

	// Return column number
	return cno;
}


// Read keypad key
char KeyScan(void)
{
	int rno,cno,keyv;   // Row, column and key values

	// Wait for key press
	while(ColScan());

	// Find row number
	rno=RowCheck();

	// Find column number
	cno=ColCheck();

	// Get key value from keypad table
	keyv=kpmLUT[rno][cno];

	// Wait for key release
	while(!ColScan());

	// Return key value
	return keyv;
}
