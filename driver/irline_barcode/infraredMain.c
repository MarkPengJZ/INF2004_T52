// MUST BE RUN FROM DEV CONSOLE: type Code and it will launch vsCode but you can build.
// Build settings can be found in the bottom bar of vscode. It should be
// using [GCC 10.3.1 arm-none-eabi] as the active kit and [infraredMain] as the the build target.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
// Pico libraries
#include "pico/types.h" // Timers
#include "pico/stdlib.h"
#include "pico/stdio.h"

#include "infraredMain.h"

// Define use of Pico LED
// #define GPIO_PIN_LED 25

// Define Infrared Sensor Needs
#define GPIO_PIN_INFRARED_IN1 11

// Declarations
volatile bool infraFlag = true;
volatile bool readyToStartBarcodeState = false;
volatile int arrayVar = 0;         // Used in main loop to check 1 char scanned // To-do: Make it dynamic
volatile int charStartEndCheck[9]; // Used in checking for * symbol
volatile int timeChanges[9];       // Stores the differences in timings

// Character Logic Variables
volatile int startReading = 0;
int varcharStartEndStar[9];    // Used for only comparing * (starting/ ending characters)
volatile int countStar = 0;    // Used to count the number of stars
volatile int compareQ = 0;     // Used in remembering the start index of charStartEndCheck (denoising loop through algo)
char varCharASCII = '~';       // Used for storing main data character
char finalString[22];          // 1 used for *, 1 to store ending char '/0' if not buffer overflow; 20 char storage
char sanitizedFinalString[20]; // Used for returning the final string without the *
const int TIMEOUT = 1200;      // If scan of barcode is past this threshold in ms (1200ms = 1.2s), cancel reading of barcode. // Prevents softlock

// Denoising
bool arrayStarNotMatch = true; // true is not match, false is match

// Backwards search
bool isBackwards = false;

// CODE39 Char Arrays. 1 is LONG bar. 0 is SHORT.
const int charStartEndStar[9] = {0, 1, 0, 0, 1, 0, 1, 0, 0};
const int charSpace[9] = {0, 1, 1, 0, 0, 0, 1, 0, 0};
const int letterA[9] = {1, 0, 0, 0, 0, 1, 0, 0, 1};
const int letterB[9] = {0, 0, 1, 0, 0, 1, 0, 0, 1};
const int letterC[9] = {1, 0, 1, 0, 0, 1, 0, 0, 0};
const int letterD[9] = {0, 0, 0, 0, 1, 1, 0, 0, 1};
const int letterE[9] = {1, 0, 0, 0, 1, 1, 0, 0, 0};
const int letterF[9] = {0, 0, 1, 0, 1, 1, 0, 0, 0};
const int letterG[9] = {0, 0, 0, 0, 0, 1, 1, 0, 1};
const int letterH[9] = {1, 0, 0, 0, 0, 1, 1, 0, 0};
const int letterI[9] = {0, 0, 1, 0, 0, 1, 1, 0, 0};
const int letterJ[9] = {0, 0, 0, 0, 1, 1, 1, 0, 0};
const int letterK[9] = {1, 0, 0, 0, 0, 0, 0, 1, 1};
const int letterL[9] = {0, 0, 1, 0, 0, 0, 0, 1, 1};
const int letterM[9] = {1, 0, 1, 0, 0, 0, 0, 1, 0};
const int letterN[9] = {0, 0, 0, 0, 1, 0, 0, 1, 1};
const int letterO[9] = {1, 0, 0, 0, 1, 0, 0, 1, 0};
const int letterP[9] = {0, 0, 1, 0, 1, 0, 0, 1, 0};
const int letterQ[9] = {0, 0, 0, 0, 0, 0, 1, 1, 1};
const int letterR[9] = {1, 0, 0, 0, 0, 0, 1, 1, 0};
const int letterS[9] = {0, 0, 1, 0, 0, 0, 1, 1, 0};
const int letterT[9] = {0, 0, 0, 0, 1, 0, 1, 1, 0};
const int letterU[9] = {1, 1, 0, 0, 0, 0, 0, 0, 1};
const int letterV[9] = {0, 1, 1, 0, 0, 0, 0, 0, 1};
const int letterW[9] = {1, 1, 1, 0, 0, 0, 0, 0, 0};
const int letterX[9] = {0, 1, 0, 0, 1, 0, 0, 0, 1};
const int letterY[9] = {1, 1, 0, 0, 1, 0, 0, 0, 0};
const int letterZ[9] = {0, 1, 1, 0, 1, 0, 0, 0, 0};
const int charDash[9] = {0, 1, 0, 0, 0, 0, 1, 0, 1};
const int charPeriod[9] = {1, 1, 0, 0, 0, 0, 1, 0, 0};
const int charDollar[9] = {0, 1, 0, 1, 0, 1, 0, 0, 0};
const int charBackslash[9] = {0, 1, 0, 1, 0, 0, 0, 1, 0};
const int charPlus[9] = {0, 1, 0, 0, 0, 1, 0, 1, 0};
const int charPercentage[9] = {0, 0, 0, 1, 0, 1, 0, 1, 0};
const int no0[9] = {0, 0, 0, 1, 1, 0, 1, 0, 0};
const int no1[9] = {1, 0, 0, 1, 0, 0, 0, 0, 1};
const int no2[9] = {0, 0, 1, 1, 0, 0, 0, 0, 1};
const int no3[9] = {1, 0, 1, 1, 0, 0, 0, 0, 0};
const int no4[9] = {0, 0, 0, 1, 1, 0, 0, 0, 1};
const int no5[9] = {1, 0, 0, 1, 1, 0, 0, 0, 0};
const int no6[9] = {0, 0, 1, 1, 1, 0, 0, 0, 0};
const int no7[9] = {0, 0, 0, 1, 0, 0, 1, 0, 1};
const int no8[9] = {1, 0, 0, 1, 0, 0, 1, 0, 0};
const int no9[9] = {0, 0, 1, 1, 0, 0, 1, 0, 0};

int compareArrays(int a[], const int b[])
{
    int i;
    for (i = 0; i < 9; i++)
    {
        if (a[i] != b[i])
            return 1; // If array not equal, return 1
    }
    return 0; // If array equal, return 0
}

void initInfrared(void)
{
    printf("[Encoder] Initialization started \n");

    // Initialization
    gpio_init(GPIO_PIN_INFRARED_IN1);
    gpio_set_dir(GPIO_PIN_INFRARED_IN1, GPIO_IN);

    printf("[Encoder] Initialization done \n");
}

int readyToStartBarcode()
{
    // WHITE BAR (probably means start of barcode) DETECTED
    if (gpio_get(GPIO_PIN_INFRARED_IN1) == 0)
    {
        readyToStartBarcodeState = true;
        return 1; // For integration into main
    };
}
void INFRARED_scanning()
{
    /** SCANNING OF B/W **/
    if (gpio_get(GPIO_PIN_INFRARED_IN1) == 1) // BLACK BAR
    {
        if (infraFlag == true)
        {
            infraFlag = false;
            sleep_us(100);                                     // Delay of at least 10 microseconds NEEDS to be here otherwise it wont work
            charStartEndCheck[arrayVar] = time_us_32() / 1000; // Get raw time value from system
            arrayVar += 1;                                     // arrayVar changes on each color change (B->W or W->B)
        }
        // printf("Here is the current time in miliseconds: %i\n", time_us_32()/1000);
    }
    else
    {
        if (infraFlag == false)
        {
            infraFlag = true;
            sleep_us(100);                                     // Delay of at least 10 microseconds NEEDS to be here otherwise it wont work
            charStartEndCheck[arrayVar] = time_us_32() / 1000; // Get raw time value from system
            arrayVar += 1;                                     // arrayVar changes on each color change (B->W or W->B)
        }
    }
}

bool INFRARED_readyToReturnChar()
{
    if (countStar >= 2)
    {
        // Return star to not found
        return true;
    }
    else
        return false;
}
const char *INFRARED_returnChar()
{
    /* WHEN COUNTSTAR = 2, PRINT THE FINAL STRING HERE */
    // printf("\n\n YOUR FINAL READING IS ON THE FOLLOWING LINE: \n \033[1;31m %s \033[0m", finalString);
    int tempCount = 0;
    int tempCountStar = 0;
    for (int i = 0; finalString[i] != '\0'; ++i)
    {
        if (finalString[i] != '*')
        {
            finalString[tempCount] = finalString[i];
            tempCount++;
        }
        if (finalString[i] == '*')
        {
            tempCountStar++;
        }
        if (tempCountStar == 2)
        {
            finalString[tempCount] = '\0';
        }
    }
    // printf("tempFinal String ->%s<- \t finalString ->%s<-\n", sanitizedFinalString, finalString);
    return finalString;
}
void INFRARED_resetForNewString()
{
    countStar = 0;                    // Reset count star
    varCharASCII = '~';               // Reset variable
    arrayStarNotMatch = true;         // Set back star as not found
    isBackwards = false;              // If it was scanned backwards, reset it
    strcpy(finalString, "");          // Clear contents of string
    strcpy(sanitizedFinalString, ""); // Clear contents of cleaned string
    readyToStartBarcodeState = false; // Exit barcode state
}

void INFRARED_decodeCharTree()
{

    if (isBackwards)
    {
        /* START CHARACTER SORTING FOR BACKWARDS */
        // If first number in data section of array starts with 1
        // KRAHOE185MC3U.YW
        if (varcharStartEndStar[0])
        { // Currently using varcharStartEndStar,
            if (varcharStartEndStar[8])
            {
                // U1AK = KA1U
                if (compareArray(varcharStartEndStar, letterU) == 0)
                {
                    varCharASCII = 'K';
                }
                else if (compareArray(varcharStartEndStar, no1) == 0)
                {
                    varCharASCII = 'A';
                }
                else if (compareArray(varcharStartEndStar, letterA) == 0)
                {
                    varCharASCII = '1';
                }
                else
                {
                    varCharASCII = 'U';
                }
            }

            else if (varcharStartEndStar[2])
            {
                // W3CM = QG7-
                if (compareArray(varcharStartEndStar, letterW) == 0)
                {
                    varCharASCII = 'Q';
                }
                else if (compareArray(varcharStartEndStar, no3) == 0)
                {
                    varCharASCII = 'G';
                }
                else if (compareArray(varcharStartEndStar, letterC) == 0)
                {
                    varCharASCII = '7';
                }
                else
                {
                    varCharASCII = '-';
                }
            }
            else if (varcharStartEndStar[6])
            {
                //.8HR = LB2V
                if (compareArray(varcharStartEndStar, charPeriod) == 0)
                {
                    varCharASCII = 'L';
                }
                else if (compareArray(varcharStartEndStar, no8) == 0)
                {
                    varCharASCII = 'B';
                }
                else if (compareArray(varcharStartEndStar, letterH) == 0)
                {
                    varCharASCII = '2';
                }
                else
                {
                    varCharASCII = 'V';
                }
            }
            else
            {
                // OE5Y = X4DN
                if (compareArray(varcharStartEndStar, letterO) == 0)
                {
                    varCharASCII = 'X';
                }
                else if (compareArray(varcharStartEndStar, letterE) == 0)
                {
                    varCharASCII = '4';
                }
                else if (compareArray(varcharStartEndStar, no5) == 0)
                {
                    varCharASCII = 'D';
                }
                else
                {
                    varCharASCII = 'N';
                }
            }
        }

        // If first number in data section of array starts with 0
        // QGNTDJ7%40LSBIPF296-+X*/$VZ SPACE
        else if (!varcharStartEndStar[0])
        {
            // VX-2BL47DNGQ
            if (varcharStartEndStar[8]) // Ending number 1
            {
                if (varcharStartEndStar[2])
                {
                    // V2BL = RH8.
                    if (compareArray(varcharStartEndStar, letterV) == 0)
                    {
                        varCharASCII = 'R';
                    }
                    else if (compareArray(varcharStartEndStar, no2) == 0)
                    {
                        varCharASCII = 'H';
                    }
                    else if (compareArray(varcharStartEndStar, letterB) == 0)
                    {
                        varCharASCII = '8';
                    }
                    else
                    {
                        varCharASCII = '.';
                    }
                }
                else if (varcharStartEndStar[6])
                {
                    //-7GQ = MC3W
                    if (compareArray(varcharStartEndStar, charDash) == 0)
                    {
                        varCharASCII = 'M';
                    }
                    else if (compareArray(varcharStartEndStar, no7) == 0)
                    {
                        varCharASCII = 'C';
                    }
                    else if (compareArray(varcharStartEndStar, letterG) == 0)
                    {
                        varCharASCII = '3';
                    }
                    else
                    {
                        varCharASCII = 'W';
                    }
                }
                else
                {
                    // X4DN = OE5Y
                    if (compareArray(varcharStartEndStar, letterX) == 0)
                    {
                        varCharASCII = 'O';
                    }
                    else if (compareArray(varcharStartEndStar, no4) == 0)
                    {
                        varCharASCII = 'E';
                    }
                    else if (compareArray(varcharStartEndStar, letterD) == 0)
                    {
                        varCharASCII = '5';
                    }
                    else
                    {
                        varCharASCII = 'Y';
                    }
                }
            }
            else // Ending number 0
            {
                // Z$/*+69FPIS0%JT SPACE <0>
                if (varcharStartEndStar[1])
                {
                    // Z$/+ SPACE = T%+/ S
                    if (varcharStartEndStar[2])
                    {
                        // Z SPACE
                        if (compareArray(varcharStartEndStar, letterZ) == 0)
                        {
                            varCharASCII = 'T';
                        }
                        else
                        {
                            varCharASCII = 'S';
                        }
                    }
                    else
                    {
                        // $/*+
                        if (compareArray(varcharStartEndStar, charStartEndStar) == 0)
                        {
                            varCharASCII = 'P';
                        }
                        else if (compareArray(varcharStartEndStar, charDollar) == 0)
                        {
                            varCharASCII = '%';
                        }
                        else if (compareArray(varcharStartEndStar, charBackslash) == 0)
                        {
                            varCharASCII = '+';
                        }
                        else
                        {
                            varCharASCII = '/';
                        }
                    }
                }
                else if (varcharStartEndStar[2])
                {
                    // 69FPIS = JI0*9 SPACE
                    if (varcharStartEndStar[4])
                    {
                        // 6FP = J0*
                        if (compareArray(varcharStartEndStar, no6) == 0)
                        {
                            varCharASCII = 'J';
                        }
                        else if (compareArray(varcharStartEndStar, letterF) == 0)
                        {
                            varCharASCII = '0';
                        }
                        else
                        {
                            varCharASCII = '*';
                        }
                    }
                    else
                    {
                        // 9IS = I9 SPACE
                        if (compareArray(varcharStartEndStar, no9) == 0)
                        {
                            varCharASCII = 'I';
                        }
                        else if (compareArray(varcharStartEndStar, letterI) == 0)
                        {
                            varCharASCII = '9';
                        }
                        else
                        {
                            varCharASCII = ' ';
                        }
                    }
                }
                else
                {
                    // 0%JT = F$6Z
                    if (compareArray(varcharStartEndStar, no0) == 0)
                    {
                        varCharASCII = 'F';
                    }
                    else if (compareArray(varcharStartEndStar, charPercentage) == 0)
                    {
                        varCharASCII = '$';
                    }
                    else if (compareArray(varcharStartEndStar, letterJ) == 0)
                    {
                        varCharASCII = '6';
                    }
                    else
                    {
                        varCharASCII = 'Z';
                    }
                }
            }
        }
        /* END CHARACTER SORTING */
    }

    /* if isBackwards == true sorting table*/
    else
    {
        /* START CHARACTER SORTING */
        // If first number in data section of array starts with 1
        // KRAHOE185MC3U.YW
        if (varcharStartEndStar[0])
        { // Currently using varcharStartEndStar,
            if (varcharStartEndStar[8])
            {
                // U1AK
                if (compareArray(varcharStartEndStar, letterU) == 0)
                {
                    varCharASCII = 'U';
                }
                else if (compareArray(varcharStartEndStar, no1) == 0)
                {
                    varCharASCII = '1';
                }
                else if (compareArray(varcharStartEndStar, letterA) == 0)
                {
                    varCharASCII = 'A';
                }
                else
                {
                    varCharASCII = 'K';
                }
            }

            else if (varcharStartEndStar[2])
            {
                // W3CM
                if (compareArray(varcharStartEndStar, letterW) == 0)
                {
                    varCharASCII = 'W';
                }
                else if (compareArray(varcharStartEndStar, no3) == 0)
                {
                    varCharASCII = '3';
                }
                else if (compareArray(varcharStartEndStar, letterC) == 0)
                {
                    varCharASCII = 'C';
                }
                else
                {
                    varCharASCII = 'M';
                }
            }
            else if (varcharStartEndStar[6])
            {
                //.8HR
                if (compareArray(varcharStartEndStar, charPeriod) == 0)
                {
                    varCharASCII = '.';
                }
                else if (compareArray(varcharStartEndStar, no8) == 0)
                {
                    varCharASCII = '8';
                }
                else if (compareArray(varcharStartEndStar, letterH) == 0)
                {
                    varCharASCII = 'H';
                }
                else
                {
                    varCharASCII = 'R';
                }
            }
            else
            {
                // OE5Y
                if (compareArray(varcharStartEndStar, letterO) == 0)
                {
                    varCharASCII = 'O';
                }
                else if (compareArray(varcharStartEndStar, letterE) == 0)
                {
                    varCharASCII = 'E';
                }
                else if (compareArray(varcharStartEndStar, no5) == 0)
                {
                    varCharASCII = '5';
                }
                else
                {
                    varCharASCII = 'Y';
                }
            }
        }

        // If first number in data section of array starts with 0
        // QGNTDJ7%40LSBIPF296-+X*/$VZ SPACE
        else if (!varcharStartEndStar[0])
        {
            // VX-2BL47DNGQ
            if (varcharStartEndStar[8]) // Ending number 1
            {
                if (varcharStartEndStar[2])
                {
                    // V2BL
                    if (compareArray(varcharStartEndStar, letterV) == 0)
                    {
                        varCharASCII = 'V';
                    }
                    else if (compareArray(varcharStartEndStar, no2) == 0)
                    {
                        varCharASCII = '2';
                    }
                    else if (compareArray(varcharStartEndStar, letterB) == 0)
                    {
                        varCharASCII = 'B';
                    }
                    else
                    {
                        varCharASCII = 'L';
                    }
                }
                else if (varcharStartEndStar[6])
                {
                    //-7GQ
                    if (compareArray(varcharStartEndStar, charDash) == 0)
                    {
                        varCharASCII = '-';
                    }
                    else if (compareArray(varcharStartEndStar, no7) == 0)
                    {
                        varCharASCII = '7';
                    }
                    else if (compareArray(varcharStartEndStar, letterG) == 0)
                    {
                        varCharASCII = 'G';
                    }
                    else
                    {
                        varCharASCII = 'Q';
                    }
                }
                else
                {
                    // X4DN
                    if (compareArray(varcharStartEndStar, letterX) == 0)
                    {
                        varCharASCII = 'X';
                    }
                    else if (compareArray(varcharStartEndStar, no4) == 0)
                    {
                        varCharASCII = '4';
                    }
                    else if (compareArray(varcharStartEndStar, letterD) == 0)
                    {
                        varCharASCII = 'D';
                    }
                    else
                    {
                        varCharASCII = 'N';
                    }
                }
            }
            else // Ending number 0
            {
                // Z$/*+69FPIS0%JT SPACE <0>
                if (varcharStartEndStar[1])
                {
                    // Z$/+ SPACE
                    if (varcharStartEndStar[2])
                    {
                        // Z SPACE
                        if (compareArray(varcharStartEndStar, letterZ) == 0)
                        {
                            varCharASCII = 'Z';
                        }
                        else
                        {
                            varCharASCII = ' ';
                        }
                    }
                    else
                    {
                        // $/*+
                        if (compareArray(varcharStartEndStar, charStartEndStar) == 0)
                        {
                            varCharASCII = '*';
                        }
                        else if (compareArray(varcharStartEndStar, charDollar) == 0)
                        {
                            varCharASCII = '$';
                        }
                        else if (compareArray(varcharStartEndStar, charBackslash) == 0)
                        {
                            varCharASCII = '/';
                        }
                        else
                        {
                            varCharASCII = '+';
                        }
                    }
                }
                else if (varcharStartEndStar[2])
                {
                    // 69FPIS
                    if (varcharStartEndStar[4])
                    {
                        // 6FP
                        if (compareArray(varcharStartEndStar, no6) == 0)
                        {
                            varCharASCII = '6';
                        }
                        else if (compareArray(varcharStartEndStar, letterF) == 0)
                        {
                            varCharASCII = 'F';
                        }
                        else
                        {
                            varCharASCII = 'P';
                        }
                    }
                    else
                    {
                        // 9IS
                        if (compareArray(varcharStartEndStar, no9) == 0)
                        {
                            varCharASCII = '9';
                        }
                        else if (compareArray(varcharStartEndStar, letterI) == 0)
                        {
                            varCharASCII = 'I';
                        }
                        else
                        {
                            varCharASCII = 'S';
                        }
                    }
                }
                else
                {
                    // 0%JT
                    if (compareArray(varcharStartEndStar, no0) == 0)
                    {
                        varCharASCII = '0';
                    }
                    else if (compareArray(varcharStartEndStar, charPercentage) == 0)
                    {
                        varCharASCII = '%';
                    }
                    else if (compareArray(varcharStartEndStar, letterJ) == 0)
                    {
                        varCharASCII = 'J';
                    }
                    else
                    {
                        varCharASCII = 'T';
                    }
                }
            }
        }
        /* END CHARACTER SORTING */
    }

    // Concatenating the characters into var finalString
    if (!arrayStarNotMatch)
        strncat(finalString, &varCharASCII, 1);
}

bool INFRARED_oneCharRead()
{
    return (arrayVar == 10);
}
void INFRARED_sortingTimings()
{

    /* To sort and find out the diff between each time interval */
    for (int i = 0; i < 9; i++)
    {
        timeChanges[i] = charStartEndCheck[i + 1] - charStartEndCheck[i]; // Differences in each timing captured
        // printf("T%i in miliseconds corrected: %i (%i) \n", i, charStartEndCheck[i], timeChanges[i]);
        if (timeChanges[i] > TIMEOUT)
        {
            printf("Barcode reading cancelled, stayed at one spot for too long.");
            INFRARED_resetForNewString();
            break;
        }
    }

    /* Find if its long or short bars */
    int h1 = INT32_MIN; // Highest Timing, 1st long bar
    int h2 = INT32_MIN; // 2nd Highest Timing, 2nd long bar
    int h3 = INT32_MIN; // 3rd Highest Timing, 3rd long bar
    int h1i, h2i, h3i = 0;
    for (int i = 0; i < 9; i++)
    {
        if (timeChanges[i] > h1)
        {
            h3i = h2i;
            h2i = h1i;
            h1i = i;

            h3 = h2;
            h2 = h1;
            h1 = timeChanges[i];
        }
        else if (timeChanges[i] > h2)
        {
            h3i = h2i;
            h2i = i;

            h3 = h2;
            h2 = timeChanges[i];
        }
        else if (timeChanges[i] > h3)
        {
            h3i = i;
            h3 = timeChanges[i];
        }
    }

    /* Remembers the position of the long bars in respect to how the timings came in */
    for (int i = 0; i < 9; i++)
    {
        if (i % 2)
        {
            if (i == h1i || i == h2i || i == h3i)
            {
                varcharStartEndStar[i] = 1;
            }
            else
            {
                varcharStartEndStar[i] = 0;
            }
        }
        else
        {
            if (i == h1i || i == h2i || i == h3i)
            {
                varcharStartEndStar[i] = 1;
            }
            else
            {
                varcharStartEndStar[i] = 0;
            }
        }
    }

    // If character = * || Used to know when to deliver message - By looking at if got starting and ending character
    if (compareArray(varcharStartEndStar, charStartEndStar) == 0)
    {
        arrayStarNotMatch = false; // If star is found, change bool
        if (varCharASCII != '*')
        {
            varCharASCII = '*'; // Prevent looping in case of slow reading
            if (countStar < 2)
            {
                countStar++;
            }
        }
    }
    else if (compareArray(varcharStartEndStar, letterP) == 0)
    {
        isBackwards = true;
        arrayStarNotMatch = false; // If star is found, change bool
        if (varCharASCII != '*')
        {
            varCharASCII = '*'; // Prevent looping in case of slow reading
            if (countStar < 2)
            {
                countStar++;
            }
        }
    }

    // If havent find star, everything left shift 1
    if (arrayStarNotMatch)
    {
        arrayVar = 9; // Change arrayVar to 9 instead of 10 // arrayVar -= 1;

        // Move array to the left
        for (int i = 0; i < 9; i++)
        {
            charStartEndCheck[i] = charStartEndCheck[i + 1];
        }
        charStartEndCheck[9] = 0;
    }

    // Star has been found once, return to normal scanning
    else
    {
        arrayVar = 0;
    }

    INFRARED_decodeCharTree(); // Decode char & add to finalString based on array of timings
    printf("COUNTSTARS: %d\n", countStar);
}

int main()
{
    stdio_init_all(); // For printing into PUTTY program

    // Main loop
    while (true)
    {

        // Scanning of B/W
        readyToStartBarcode(); // If white detected, used to move to barcode state in integration
        if (readyToStartBarcodeState == true)
        {
            INFRARED_scanning();
        }

        // One char has been read, sort the timings
        if (INFRARED_oneCharRead())
        {
            INFRARED_sortingTimings();
        }

        // Returning Char
        if (INFRARED_readyToReturnChar())
        {
            printf("\n\n YOUR FINAL READING IS ON THE FOLLOWING LINE: \n \033[1;31m%s\033[0m",
                   INFRARED_returnChar());
            INFRARED_resetForNewString(); // After getting final value, reset everything to be able to read a new char/string.
        }
    }
}