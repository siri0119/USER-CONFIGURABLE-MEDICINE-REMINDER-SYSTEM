// ==================================================================================
// FILE: LCD.c
// DESCRIPTION: Core driver managing 8-bit parallel Liquid Crystal Display routines
// ==================================================================================

#include<lpc21xx.h>           // Include low-level architecture peripheral register maps for NXP LPC21xx MCUs.
#include"lcd_defines.h"       // Incorporate macro mappings for LCD command hexes and controller interface pin offsets.

//delay of micro sec          // Descriptive block label highlighting the microsecond-level timing control subroutine.
void delay_us(int dlyus)      // Define low-overhead looping routine creating brief hardware microsecond pauses.
{                             // Open localized context block boundary for the microsecond delay operations.
for(dlyus*=12;dlyus>0;dlyus--); // Scale counter for 60MHz processor execution loops and decrement down to track elapsed time.
}                             // Terminate microsecond delay routine and return code control flow.

//delay of milli sec          // Descriptive block label highlighting the millisecond-level timing control subroutine.
void delay_ms(int dlyms)      // Define a standard operational loop creating multi-millisecond timing pauses.
{                             // Open context boundary block tracking the millisecond timer looping actions.
for(dlyms*=12000;dlyms>0;dlyms--); // Multiply input variable by basic clock scaling factors to block program flow for precise milliseconds.
}                             // Terminate millisecond delay function block clearing loop structures.

void WriteLCD(unsigned char cmd) // Create foundational strobe driver routing byte commands or raw character tokens straight over to hardware registers.
{                             // Open execution path context for physical bus data write sequencing.
				IOCLR0=1<<RW;//RW=0,write operation // Force Read/Write pin low inside Port 0 register to explicitly select write transmission mode.
        IOPIN0=(IOPIN0&~(0XFF<<PIN))|(cmd<<PIN); // Clear previous data pin footprints and shift incoming command byte over to specified pin alignments.
				IOSET0=1<<EN; // Drive the LCD Enable strobe pin high to flag availability of bus transmission data.
				delay_us(1); // Introduce minor operational pulse pause satisfying required setup time windows.
				IOCLR0=1<<EN; // Pull Enable strobe pin back to ground to lock transmission data into HD44780 controller inputs.
				delay_ms(2);//internal process // Pause process flow briefly to give the display panel controller time to process the internal instruction.
}                             // Close functional execution boundary for parallel data write sequences.

void CmdLCD(unsigned char cmd) // Implement an abstraction routine to forward specific instructions or display adjustments to the panel.
{                             // Begin execution tracking block for the instruction dispatch sequence.
        IOCLR0=1<<RS;//RS=0,command reg selected // Ground Register Select control line to specify incoming transfer maps to instruction register.
				WriteLCD(cmd); // Dispatch raw command token value via bus driving master writing routine.
}                             // Conclude structural block for handling basic control code updates.

//display characters          // Section marker indicating start of text literal routing methods.
void CharLCD(unsigned char ascii) // Define the character printer function translating standard ASCII values onto active line rows.
{                             // Open code frame tracking explicit singular character display actions.
	IOSET0=1<<RS;//RS=1,data reg selected // Raise the Register Select pin high to inform controller data maps to display RAM address slots.
	WriteLCD(ascii);           // Send raw byte code literal directly over bus to be written into screen RAM layout.
}                             // Close execution path frame for separate character parsing processes.

//initilization LCD           // Design section comment tracking hardware wake-up routines and initialization loops.
void InitLCD(void)            // Configure directional definitions and coordinate startup parameter sequences for the module.
{                             // Open functional driver space managing baseline configuration sweeps.
        IODIR0|=((0XFF<<PIN) | (1<<RS) | (1<<RW) | (1<<EN));//PIN,RS,RW,EN make as output pins // Establish GPIO Port 0 configuration making all data and control bus tracks function as active outputs.
        delay_ms(15);         // Maintain startup hold delay allowing physical supply currents to achieve stable working levels.
        CmdLCD(M_8BIT_1L);    // Force initial standard 8-bit bus mode selection parameter over to the target hardware controller.
        delay_ms(5);          // Introduce software pause allowing internal hardware state machines to shift modes successfully.
        CmdLCD(M_8BIT_1L);    // Issue duplicate 8-bit mode selection token to fulfill standard hardware wake sequence requirements.
        delay_us(100);        // Allocate brief delay matching processing intervals of internal controller logic.
        CmdLCD(M_8BIT_1L);    // Provide third confirmation byte sequence guaranteeing core peripheral sync states.
        CmdLCD(M_8BIT_2L);    // Initialize standard display options to operate with a multi-line format structure.
        CmdLCD(D_ON);         // Activate the character matrix screen layer to render text.
        CmdLCD(CLR);          // Erase previous display register data footprints and reset memory indices back to home position.
        CmdLCD(SHIFT_C_R);    // Configure data cursor configurations to automatically increment towards right side after each letter print.

}                             // Close master display peripheral initialization code boundary.

//display strings             // Structural separator distinguishing the string streaming functions.
void StrLCD(signed char *ptr) // Create a data streaming function to handle the print iteration of consecutive text characters.
{                             // Open loop monitoring framework evaluating standard pointer structures.
	while(*ptr)                // Construct conditional tracking loop checking until null-terminator byte '0' context matches.
	{                          // Open inner block processing data literals at sequential memory positions.
		CharLCD(*ptr++);        // Dispatch currently referenced text character to display panel and advance pointer to next memory slot.
	}                          // Terminate string block loop cycle stepping over to adjacent address entries.
}                             // Exit string tracking function clearing local workspace reference paths.
	
//display integers            // Functional division label calling attention to signed numerical base conversion structures.
void intLCD(int num)          // Implement mathematical indexing tool to parse and output multi-digit system variable integers.
{                             // Begin arithmetic translation workspace tracking transient numerical digits.
	char a[10];                // Allocate transient array buffer storage frame to stack digit symbols in inverted patterns.
	int i=0;                   // Set local index array index pointer starting off at bottom zero position.
	if(num==0)                 // Validate if input tracking parameter maps directly to simple absolute zero context.
		CharLCD('0');           // Write single literal '0' token directly onto panel when zero state matches.
	if(num<0)                  // Check if numerical value contains a negative sign prefix.
	{                          // Open logic tracking adjustments frame to output negative symbols.
		CharLCD('-');           // Print minus prefix character marker over to display screen register path.
		num=-(num);             // Convert current numerical storage metrics over to absolute positive figures.
	}                          // Conclude signed prefix adjustment conditional execution steps.
	while(num)                 // Set extraction loop dividing numerical scale down unit by unit.
	{                          // Open mod computation boundaries isolating specific individual digits.
		a[i++]=(num%10)+'0';    // Isolate single lowest digit using modulo 10 and map directly to its corresponding ASCII offset.
		num/=10;                // Divide core tracker value by ten to shift focus to next decimal column.
	}                          // Conclude current column extraction pass transitioning loops.
	for(i-=1;i>=0;i--)         // Establish reverse-traversal loop reading out from buffer array to print figures in correct order.
		CharLCD(a[i]);          // Print out translated digit code token straight into active LCD display paths.
}                             // Terminate numerical base conversion routine clean stack paths.

		//display float numbers // Header comment describing implementation of decimal point fractional conversions.
void FloatLCD(float dec,unsigned char  DP) // Build data formatter method isolating floating point components into printable strings.
{                             // Establish stack space frame mapping floating decimal arguments.
	int n,i;                   // Set tracking fields representing integer base elements and precision counter loops.
	if(dec<0)                  // Check if float value falls underneath negative line thresholds.
	{                          // Open code adjustments frame for rendering negative decimal markers.
		CharLCD('-');           // Write minus symbol character literal directly into active system display.
		dec=-(dec);             // Reverse signed property metrics tracking base scale parameter to positive territory.
	}                          // Conclude validation block checking for negative floating elements.
		n=dec;                  // Cast and assign floating metric data directly over to integer types to truncate whole part.
		intLCD(n);              // Print out isolated leading whole number block using base integer conversion tool.
		CharLCD('.');           // Print required dot character literal establishing split for fractional components.
		for(i=0; i<DP; i++)     // Execute a loop to iterate and extract individual digits up to the requested precision limit.
		{                      // Open precision calculation loop boundary context.
			dec=(dec-n)*10;      // Isolate current fraction residual balance and shift decimal rank up by multiplying by ten.
			n=dec;               // Assign newly discovered leading integer fractional unit to tracking target.
			CharLCD(n+48);       // Print computed fractional component character on display by applying ASCII numeric offset 48.
		}                      // Exit current fractional point tracking traversal loops.
}                             // Close complete execution parameters for floating display function block.
void BinLCD(unsigned int num) // Define diagnostic visualization algorithm rendering numerical metrics into raw binary tracks.
{                             // Open functional workspace tracking bitwise parsing actions.
	int i;                     // Declare loop register tracking bit offset index positions.
	for(i=7;i>=0;i--)          // Initiate descending loop checking register states from high-bit position down to zero.
	{                          // Begin processing loop tracking isolated specific data tracks.
		intLCD((num>>i)&1);     // Extract isolated bit state using rightward shift mask and forward result to LCD printer.
	}                          // Close binary bit inspection iteration loop boundary.
}                             // Terminate binary diagnostic extraction routine context.

void CustomChar(unsigned char *ch,int num) // Implement CGRAM pattern streaming routine enabling generation of specialized icons.
{                             // Open custom matrix initialization workspace context.
	int i;                     // Declare tracker counter variable guiding pattern byte transfers.
	CmdLCD(GOTO_CGRAM);        // Shift system writing focus into internal Character Generator RAM zone using macro instruction.

	for(i=0;i<num;i++)         // Create a loop to transfer the specified number of custom glyph matrix row bytes.
	{                          // Open transfer context tracking character design row data arrays.
		CharLCD(ch[i]);         // Write raw pixel row byte information to store user design definitions in controller memory.
	}                          // Complete individual row write iteration and advance custom character pattern loop index.
}                             // Terminate custom icon creation module function framing space.