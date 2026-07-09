#include<lpc21xx.h>           // Include LPC21xx header file
#include"lcd_defines.h"       // Include LCD definitions


// Microsecond delay
void delay_us(int dlyus)
{
	for(dlyus*=12;dlyus>0;dlyus--);  // Create microsecond delay
}


// Millisecond delay
void delay_ms(int dlyms)
{
	for(dlyms*=12000;dlyms>0;dlyms--);  // Create millisecond delay
}


// Write data to LCD
void WriteLCD(unsigned char cmd)
{
	IOCLR0=1<<RW;              // Set LCD to write mode

	IOPIN0=(IOPIN0&~(0XFF<<PIN))|(cmd<<PIN);  // Send data to LCD pins

	IOSET0=1<<EN;              // Set enable pin

	delay_us(1);               // Small delay

	IOCLR0=1<<EN;              // Clear enable pin

	delay_ms(2);               // Wait for LCD processing
}


// Send command to LCD
void CmdLCD(unsigned char cmd)
{
	IOCLR0=1<<RS;              // Select command register

	WriteLCD(cmd);             // Send command to LCD
}


// Display one character
void CharLCD(unsigned char ascii)
{
	IOSET0=1<<RS;              // Select data register

	WriteLCD(ascii);           // Send character to LCD
}


// Initialize LCD
void InitLCD(void)
{
	IODIR0|=((0XFF<<PIN) | (1<<RS) | (1<<RW) | (1<<EN));  // Set LCD pins as output

	delay_ms(15);              // Delay

	CmdLCD(M_8BIT_1L);         // Set 8-bit one line mode

	delay_ms(5);               // Delay

	CmdLCD(M_8BIT_1L);         // Set 8-bit one line mode

	delay_us(100);             // Small delay

	CmdLCD(M_8BIT_1L);         // Set 8-bit one line mode

	CmdLCD(M_8BIT_2L);         // Set 8-bit two line mode

	CmdLCD(D_ON);              // Turn ON LCD display

	CmdLCD(CLR);               // Clear LCD

	CmdLCD(SHIFT_C_R);         // Move cursor to right
}


// Display string
void StrLCD(signed char *ptr)
{
	while(*ptr)                // Check until null character
	{
		CharLCD(*ptr++);        // Display character
	}
}


// Display integer number
void intLCD(int num)
{
	char a[10];                // Store number digits

	int i=0;                   // Array index

	if(num==0)                 // Check number is zero
		CharLCD('0');           // Display zero

	if(num<0)                  // Check negative number
	{
		CharLCD('-');           // Display minus sign

		num=-(num);             // Make number positive
	}

	while(num)                 // Get all digits
	{
		a[i++]=(num%10)+'0';    // Store digit as character

		num/=10;                // Remove last digit
	}

	for(i-=1;i>=0;i--)         // Display digits in correct order
		CharLCD(a[i]);          // Display digit
}


// Display float number
void FloatLCD(float dec,unsigned char DP)
{
	int n,i;                   // Integer value and loop variable

	if(dec<0)                  // Check negative number
	{
		CharLCD('-');           // Display minus sign

		dec=-(dec);             // Make number positive
	}

	n=dec;                     // Get integer part

	intLCD(n);                 // Display integer part

	CharLCD('.');              // Display decimal point

	for(i=0;i<DP;i++)          // Display decimal digits
	{
		dec=(dec-n)*10;         // Get next decimal digit

		n=dec;                  // Store decimal digit

		CharLCD(n+48);          // Display decimal digit
	}
}


// Display binary number
void BinLCD(unsigned int num)
{
	int i;                     // Loop variable

	for(i=7;i>=0;i--)          // Check 8 bits
	{
		intLCD((num>>i)&1);     // Display one bit
	}
}


// Create custom character
void CustomChar(unsigned char *ch,int num)
{
	int i;                     // Loop variable

	CmdLCD(GOTO_CGRAM);        // Select CGRAM

	for(i=0;i<num;i++)         // Store custom character data
	{
		CharLCD(ch[i]);         // Send custom character data
	}
}
