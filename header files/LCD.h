// Microsecond delay
void delay_us(int dlyus);

// Millisecond delay
void delay_ms(int dlyms);

// Write data to LCD
void WriteLCD(unsigned char cmd);

// Send command to LCD
void CmdLCD(unsigned char cmd);

// Display one character
void CharLCD(unsigned char ascii);

// Initialize LCD
void InitLCD(void);

// Display string
void StrLCD(signed char *ptr);

// Display integer number
void intLCD(int num);

// Display float number
void FloatLCD(float dec,unsigned char  DP);

// Display binary number
void BinLCD(unsigned int num);

// Create custom character
void CustomChar(unsigned char *ch,int num);
