// ==================================================================================
// FILE: KPM.h
// DESCRIPTION: Function prototypes and interface boundaries for the Matrix Keypad driver
// ==================================================================================

void InitKPM(void);           // Declare the configuration routine responsible for setting the keypad row pins to output mode.
int ColScan(void);            // Prototype the continuous monitoring function that checks if any key is currently pressed by scanning column logic states.
int RowCheck(void);           // Declare the tracking loop function that sequentially grounds individual rows to isolate a pressed button.
int ColCheck(void);           // Prototype the scanning function that evaluates column input states to pinpoint the active column intersection.
char KeyScan(void);           // Declare the top-level blocking interface function that orchestrates complete keystroke discovery, debouncing, and look-up translation.