// ==================================================================================
// FILE: LCD.h
// DESCRIPTION: Function prototypes and external interface boundaries for the LCD driver
// ==================================================================================

void delay_us(int dlyus);      // Prototype the low-overhead microsecond delay function used for short hardware setup and pulse timings.
void delay_ms(int dlyms);      // Prototype the millisecond delay function used for longer display settling times and internal command execution windows.
void WriteLCD(unsigned char cmd); // Declare the foundational hardware bus write routine that toggles data and the Enable strobe pin.
void CmdLCD(unsigned char cmd); // Prototype the command abstraction function that forces the RS pin low to dispatch instruction codes.
void CharLCD(unsigned char ascii); // Declare the character display function that sets the RS pin high to print single ASCII tokens onto the screen.
void InitLCD(void);            // Prototype the hardware initialization routine that sets pin directions and sends the startup configuration sequence.
void StrLCD(signed char *ptr); // Declare the text-streaming function that loops through memory to output a null-terminated string literal.
void intLCD(int num);          // Prototype the integer translation function that decomposes numerical values into printable ASCII character sequences.
void FloatLCD(float dec,unsigned char  DP); // Declare the floating-point rendering function that handles decimal placement and precision formatting.
void BinLCD(unsigned int num); // Prototype the diagnostic debugging function that extracts and prints out numerical data in 8-bit binary format.
void CustomChar(unsigned char *ch,int num); // Declare the specialized CGRAM method used to stream custom bitmap graphics patterns straight into display RAM.