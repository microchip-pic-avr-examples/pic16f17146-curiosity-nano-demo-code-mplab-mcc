/*
© [2022] Microchip Technology Inc. and its subsidiaries.

    Subject to your compliance with these terms, you may use Microchip 
    software and any derivatives exclusively with Microchip products. 
    You are responsible for complying with 3rd party license terms  
    applicable to your use of 3rd party software (including open source  
    software) that may accompany Microchip software. SOFTWARE IS ?AS IS.? 
    NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS 
    SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT,  
    MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT 
    WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY 
    KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF 
    MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE 
    FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP?S 
    TOTAL LIABILITY ON ALL CLAIMS RELATED TO THE SOFTWARE WILL NOT 
    EXCEED AMOUNT OF FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR 
    THIS SOFTWARE.
*/

#include "mcc_generated_files/system/system.h"
#include "application.h"

#define START_OF_FRAME (0x5F)
#define END_OF_FRAME   (0xA0)

typedef enum{
    ADCC_SINGLE_ENDED_MODE,
    ADCC_DIFFERENTIAL_MODE   
}adcc_mode_t;

void GenerateWave(void);
void UpdateDac(void); 
inline void ResetTriangularWave(void);
void DisplayWave(void);
void PlotGraphInDataVisualizer(void);
void UpdateADCCMode(void);
inline adcc_mode_t GetADCCMode(void);
inline void SetADCCMode(adcc_mode_t adccMode);
inline void SetADCCPositiveChannel(adcc_posChannel_t channel);
inline void SetADCCNegativeChannel(adcc_negChannel_t channel);
void TMR4_UserInterruptHandler(void);
void TMR0_UserInterruptHandler(void);
void ADCC_UserThresholdInterruptHandler(void);

volatile bool isTimeToUpdateWave = false;
volatile bool isNewDataAvailable = false;
volatile bool isSwitchPressed = false;
uint8_t LUT_pointsCounter = NO_OF_POINTS_IN_TRIANGULAR_WAVE; // Gives the count of points in LUT
const uint8_t *LUT_ptr; // Pointer to feed the input to DAC1

// Lookup Table to generate triangular wave 
const uint8_t triangleLUT[NO_OF_POINTS_IN_TRIANGULAR_WAVE] = {
    0x4, 0x8, 0xc, 0x10, 0x14, 0x18, 0x1c, 0x20,
    0x24, 0x28, 0x2c, 0x30, 0x34, 0x38, 0x3c, 0x40,
    0x44, 0x48, 0x4c, 0x50, 0x54, 0x58, 0x5c, 0x60,
    0x64, 0x68, 0x6c, 0x70, 0x74, 0x78, 0x7c, 0x80,
    0x83, 0x87, 0x8b, 0x8f, 0x93, 0x97, 0x9b, 0x9f,
    0xa3, 0xa7, 0xab, 0xaf, 0xb3, 0xb7, 0xbb, 0xbf,
    0xc3, 0xc7, 0xcb, 0xcf, 0xd3, 0xd7, 0xdb, 0xdf,
    0xe3, 0xe7, 0xeb, 0xef, 0xf3, 0xf7, 0xfb, 0xff,
    0xfb, 0xf7, 0xf3, 0xef, 0xeb, 0xe7, 0xe3, 0xdf,
    0xdb, 0xd7, 0xd3, 0xcf, 0xcb, 0xc7, 0xc3, 0xbf,
    0xbb, 0xb7, 0xb3, 0xaf, 0xab, 0xa7, 0xa3, 0x9f,
    0x9b, 0x97, 0x93, 0x8f, 0x8b, 0x87, 0x83, 0x80,
    0x7c, 0x78, 0x74, 0x70, 0x6c, 0x68, 0x64, 0x60,
    0x5c, 0x58, 0x54, 0x50, 0x4c, 0x48, 0x44, 0x40,
    0x3c, 0x38, 0x34, 0x30, 0x2c, 0x28, 0x24, 0x20,
    0x1c, 0x18, 0x14, 0x10, 0xc, 0x8, 0x4, 0x4
};

/*
  @Description
    Configures interrupt handlers
    Configures ADCC channels
    Reset wave
    Starts ADCC trigger
    Starts generating wave
  @Preconditions
    None
  @Param
    None
  @Returns
    None      
 */
void ApplicationInit(void)
{
    // Set User Interrupt Handler for Switch Press
    Timer4.TimeoutCallbackRegister(TMR4_UserInterruptHandler);
     
    // Set User Interrupt Handler for Timer 0 overflow
    Timer0.TimeoutCallbackRegister(TMR0_UserInterruptHandler);

    // Set User Interrupt Handler for ADCC conversion complete
    ADCC_SetADTIInterruptHandler(ADCC_UserThresholdInterruptHandler);
    
    SetADCCPositiveChannel(pChannel_OPA1OUT); 
            
    SetADCCNegativeChannel(nChannel_DAC2OUT);
    
    ResetTriangularWave();
       
    Timer2.Start(); // Start ADCC triggering 
    
    Timer0.Start(); // Start updating DAC
}

/*
  @Description
   Generates and display the wave and also updates ADCC mode.
  @Preconditions
    None
  @Param
    None
  @Returns
    None      
 */
void Application(void) 
{
    GenerateWave();
    
    DisplayWave();
    
    UpdateADCCMode(); 
}

/*
  @Description
    Updates DAC value periodically as per configured time. 
    Frequency of the generated signal = 1/ (total number of points in one cycle * Timer0 period) 
    1Hz Triangular wave is generated with 128 points and Timer0 period = 7.8 msec
  @Preconditions
    None
  @Param
    None
  @Returns
    None      
 */
void GenerateWave(void)
{
    if (isTimeToUpdateWave) 
    {
        isTimeToUpdateWave = false;

        UpdateDac();
        // DAC output is connected to OPA which amplifies the wave. 
    }
}

/*
  @Description
   Changes the DAC register to generate a wave. 
  @Preconditions
    None
  @Param
    None
  @Returns
    None      
 */
void UpdateDac(void) 
{  
    DAC1_SetOutput(*(LUT_ptr++));
    LUT_pointsCounter--;
    
    if (LUT_pointsCounter <= 0) // If one wave cycle is completed
    {
        ResetTriangularWave();
    }
}

/*
  @Description
    Reset the wave cycle to the first point of triangular wave. Updates point counter.
  @Preconditions
    None
  @Param
    None
  @Returns
    None      
 */
inline void ResetTriangularWave(void)
{
    // Reset the wave cycle to the first point of triangular wave
    LUT_pointsCounter = NO_OF_POINTS_IN_TRIANGULAR_WAVE;
    LUT_ptr = triangleLUT;
}

/*
  @Description
    Update the wave when new data is available.
  @Preconditions
    None
  @Param
    None
  @Returns
    None      
 */
void DisplayWave(void)
{
    if (isNewDataAvailable) 
    {
        isNewDataAvailable = false;
        
        PlotGraphInDataVisualizer();                    
    }
}

/*
  @Description
    Plot the wave on Data Visualizer using UART
  @Preconditions
    None
  @Param
    None
  @Returns
    None      
 */
void PlotGraphInDataVisualizer(void)
{
    while (!(UART1.IsTxReady()));
    UART1.Write(START_OF_FRAME);
    
    while (!(UART1.IsTxReady()));
    UART1.Write((uint8_t)ADCC_GetFilterValue()); // Send low byte
    
    while (!(UART1.IsTxReady()));
    UART1.Write((uint8_t)(ADCC_GetFilterValue() >> 8)); // Send high byte
    
    while (!(UART1.IsTxReady()));
    UART1.Write(END_OF_FRAME);
}

/*
  @Description
   Toggles ADCC mode between single ended and differential on switch press.
   In single ended mode, ADCC input is the triangular wave generated by DAC1 and amplified by OPA.
   In differential mode, ADCC input is the differential voltage of triangular wave and the offset 
   voltage of 1.024V provided by DAC2.
  @Preconditions
    None
  @Param
    None
  @Returns
    None      
 */ 
void UpdateADCCMode(void)
{
    if (isSwitchPressed)
    {
        isSwitchPressed = false;

        LED0_Toggle(); 

        // Toggle ADCC mode between single ended and differential
        if (GetADCCMode() == ADCC_DIFFERENTIAL_MODE)
        {
            SetADCCMode(ADCC_SINGLE_ENDED_MODE);
        }
        else 
        {
            SetADCCMode(ADCC_DIFFERENTIAL_MODE);
        }

        ResetTriangularWave();
    }
}

/*
  @Description
    Returns ADCC mode
  @Preconditions
    None
  @Param
    None
  @Returns
    ADCC mode     
 */
inline adcc_mode_t GetADCCMode(void)
{
    if (ADCON0bits.IC == ADCC_DIFFERENTIAL_MODE)
    {
        return ADCC_DIFFERENTIAL_MODE;
    }
    else 
    {
        return ADCC_SINGLE_ENDED_MODE;
    }
}

/*
  @Description
    Set ADCC mode
  @Preconditions
    None
  @Param
    adccMode - ADCC mode
  @Returns
    None      
 */
inline void SetADCCMode(adcc_mode_t adccMode)
{
    ADCON0bits.IC = adccMode;
}

/*
  @Description
    Set ADCC positive channel
  @Preconditions
    None
  @Param
    channel - ADCC positive channel
  @Returns
    None      
 */
inline void SetADCCPositiveChannel(adcc_posChannel_t channel)
{
    ADPCH = channel;
}

/*
  @Description
    Set ADCC negative channel
  @Preconditions
    None
  @Param
    channel - ADCC negative channel
  @Returns
    None      
 */
inline void SetADCCNegativeChannel(adcc_negChannel_t channel)
{
    ADNCH = channel;
}

/*
  @Description
    Interrupt handler routine for Timer0 overflow.
  @Preconditions
    None
  @Param
    None
  @Returns
    None      
 */
void TMR0_UserInterruptHandler(void) 
{
    isTimeToUpdateWave = true;
}

/*
  @Description
    Interrupt handler routine for ADCC conversion upon Threshold interrupt.
    ADCC is auto-triggered by Timer2 every 2ms.
  @Preconditions
    None
  @Param
    None
  @Returns
    None      
 */
void ADCC_UserThresholdInterruptHandler(void) 
{
    isNewDataAvailable = true;
}

/*
  @Description
    Interrupt handler routine for Timer4 overflow.
    Interrupt occurs on switch press.
  @Preconditions
    None
  @Param
    None
  @Returns
    None      
 */
void TMR4_UserInterruptHandler(void) 
{
    isSwitchPressed = true;
}