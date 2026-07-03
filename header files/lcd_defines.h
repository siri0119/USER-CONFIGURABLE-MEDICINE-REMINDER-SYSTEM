// ==================================================================================
// FILE: lcd_defines.h
// DESCRIPTION: Hardware pin maps and HD44780 controller command macro constants
// ==================================================================================

#define PIN 8    //lcd data pins d0(0.8) t0 d7(0.15) // Define the base bit offset shift factor representing the start of the 8-bit parallel data bus lines on Port 0.
#define RS 18    //register select 0.16 // Assign hardware pin channel index 18 mapping the processor connection directly to the LCD Register Select pin.
#define RW 17    // read / write 0.17 // Map macro identifier to bit offset 17 locating the physical Read/Write hardware line track on Port 0.
#define EN 19   //Enable 0.18 // Associate macro symbol with bit position 19 representing the data strobe execution Enable pin connection.

// lcd commands       // Section separator grouping standard operations codes interpreted by the HD44780 display processor.

#define CLR 0x01 // clear lcd // Establish hex instruction command 0x01 designated to wipe out DDRAM contents and restore cursor position.
#define RET 0x02 //return curser to home // Define control instruction 0x02 forcing the screen cursor back to the first line's starting address index.
#define D_OFF 0x08 //display off // Specify mode byte sequence 0x08 used to completely shut off the pixel layers without clearing character memory.
#define D_ON 0x0c //display on // Declare instruction constant 0x0C intended to reactivate display data rendering while masking the cursor line.
#define D_ON_C_ON 0x0e //display on curser on // Set configuration constant 0x0E activating the panel display layer along with a solid visible cursor line.
#define D_ON_C_BLK 0x0f //display on curser blink // Map display command code 0x0F that enables panel text tracking combined with a blinking cursor matrix.

//modes of operation  // Label group identifying bus width adjustments and standard layout row rendering constants.
#define M_8BIT_1L 0x30 //mode 8 bit 1 line // Map initialization sequence 0x30 to configure an 8-bit wide parallel data bus across a single text row.
#define M_8BIT_2L 0x38 //mode 8bit 2 line // Map setup hex instruction 0x38 selecting a wide 8-bit bus split spanning across two standard display lines.
#define M_4BIT_1L 0x20 //mode 4bit 1 line // Set nibble configuration metric 0x20 configuring space-saving 4-bit bus lines for a singular line layout.
#define M_4BIT_2L 0x28 //mode 4bit 2 line // Establish command index 0x28 shifting hardware operation down to a 4-bit narrow bus crossing dual grid rows.

//lines and positions // Boundary marker highlighting memory pointers that jump the cursor to explicit terminal locations.
#define GOTO_L1_POSN0 0x80 // Establish RAM memory address constant 0x80 targeting the absolute beginning position of the first text row.
#define GOTO_L2_POSN0 0xc0 // Define base offset address 0xC0 steering display pointers over to the start of the second text row line.
#define GOTO_L3_POSN0 0x94 // Assign tracking memory pointer 0x94 targeting the default layout beginning for extended line row three.
#define GOTO_L4_POSN0 0xd4 // Define baseline target register address 0xD4 navigating the visual tracking path to the fourth display line.
#define GOTO_CGRAM 0x40    // Map starting RAM index value 0x40 shifting data stream inputs to configure custom graphic characters.
#define SHIFT_C_R 0x06     // Set macro command 0x06 establishing entry settings to step the cursor increment rightward automatically.