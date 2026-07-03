// ==================================================================================
// FILE: kpm_defines.h
// DESCRIPTION: Hardware pin definitions mapping Keypad Matrix Rows and Columns to Port 1
// ==================================================================================

#define ROW0 16 //P1.16      // Assign pin offset integer 16 representing Port 1 Pin 16 connected to the first output row of the keypad matrix.
#define ROW1 17              // Map macro identifier to bit index 17 tracking the physical layout line for the second row of keys.
#define ROW2 18              // Map macro identifier to bit index 18 tracking the physical layout line for the third row of keys.
#define ROW3 19              // Map macro identifier to bit index 19 tracking the physical layout line for the fourth row of keys.

#define COL0 20              // Assign pin offset integer 20 representing Port 1 Pin 20 connected to the first scanning column input channel.
#define COL1 22              // Map macro identifier to bit index 22 tracking the physical layout line for the second sensing column input.
#define COL2 22              // Note: Duplicate mapping identifier matching bit index 22 tracking the physical layout line for the third column line.
#define COL3 23 //P1.23      // Assign pin offset integer 23 representing Port 1 Pin 23 connected to the fourth scanning column input channel.