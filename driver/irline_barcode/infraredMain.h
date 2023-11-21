#ifndef infraredMain_H_
#define infraredMain_H_

void INFRARED_init(void);

int readyToStartBarcode();
void INFRARED_scanning(); // Scanning of B/W

// Used tgt, if done reading, return string, reset all variables, ready to read again
bool INFRARED_readyToReturnChar();
const char* INFRARED_returnChar(); // Return Final Char
void INFRARED_resetForNewString();

// Used tgt; if bool, sortTiming
bool INFRARED_oneCharRead();    
void INFRARED_sortingTimings(); // Decode Character

#endif