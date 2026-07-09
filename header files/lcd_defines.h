// LCD data pin starting position
#define PIN 8    //lcd data pins d0(0.8) t0 d7(0.15)

// LCD register select pin
#define RS 18    //register select 0.16

// LCD read/write pin
#define RW 17    // read / write 0.17

// LCD enable pin
#define EN 19   //Enable 0.18


// LCD commands

// Clear LCD
#define CLR 0x01 // clear lcd

// Return cursor to home
#define RET 0x02 //return curser to home

// Turn display OFF
#define D_OFF 0x08 //display off

// Turn display ON
#define D_ON 0x0c //display on

// Turn display and cursor ON
#define D_ON_C_ON 0x0e //display on curser on

// Turn display, cursor and blink ON
#define D_ON_C_BLK 0x0f //display on curser blink


// LCD operation modes

// 8-bit one line mode
#define M_8BIT_1L 0x30 //mode 8 bit 1 line

// 8-bit two line mode
#define M_8BIT_2L 0x38 //mode 8bit 2 line

// 4-bit one line mode
#define M_4BIT_1L 0x20 //mode 4bit 1 line

// 4-bit two line mode
#define M_4BIT_2L 0x28 //mode 4bit 2 line


// LCD line positions

// First line starting position
#define GOTO_L1_POSN0 0x80

// Second line starting position
#define GOTO_L2_POSN0 0xc0

// Third line starting position
#define GOTO_L3_POSN0 0x94

// Fourth line starting position
#define GOTO_L4_POSN0 0xd4

// Select CGRAM
#define GOTO_CGRAM 0x40

// Move cursor to right
#define SHIFT_C_R 0x06
