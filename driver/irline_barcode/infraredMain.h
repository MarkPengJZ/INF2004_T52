#ifndef infraredMain_H_
#define infraredMain_H_

void initInfrared(void);

int checkReadyToStartBarcode();
void infraredScanning(); // Scanning of B/W

// Used tgt, if done reading, return string, reset all variables, ready to read again
bool isReadyToReturnCharacter();
const char* returnCharacter(); // Return Final Char
void resetForNewString();

// Used tgt; if bool, sortTiming
bool oneCharRead();    
void sortingTimings(); // Decode Character

#endif