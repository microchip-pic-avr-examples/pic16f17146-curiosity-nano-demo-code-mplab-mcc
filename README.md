[![MCHP](images/microchip.png)](https://www.microchip.com)

# PIC16F17146 Curiosity Nano Board Demo Code

## Introduction
This example demonstrates how a 12-bit Analog-to-Digital Converter with Computation (ADCC) peripheral works using the PIC16F17146 microcontroller in Differential and Single-Ended modes. The analog peripherals such as Operational Amplifier (OPA), Digital-to-Analog Converter (DAC) and Fixed Voltage Reference (FVR) can be internally connected to ADCC, thus eliminating the use of external connections. In this example, the on-board switch of PIC16F17146 Curiosity Nano is used to switch the operation of ADCC between single ended and differential mode. The DAC is used to generate a Triangular wave using a Look-up table with a frequency determined by the timer peripheral. The signal is amplified by the OPA and sampled by ADCC. ADCC results are displayed in a graphical format using Data Visualizer.

![objective](images/objective.png)

## Related Documentation
-	[PIC16F17146 Product Page](https://www.microchip.com/en-us/product/PIC16F17146)
-	[PIC16F17146 Data Sheet](https://www.microchip.com/DS40002343)

## Software Used
- [ MPLAB® X IDE 6.15 ](https://www.microchip.com/mplab/mplab-x-ide) or newer
- [Microchip XC8 Compiler 2.45 ](https://www.microchip.com/mplab/compilers) or newer
- [MPLAB® Code Configurator (MCC) 5.3.7 ](https://www.microchip.com/mplab/mplab-code-configurator) or newer
- [Microchip PIC16F1xxxx Series Device Support (DFP) 1.22.376 ](https://packs.download.microchip.com/) or newer

## Hardware Used
-	[PIC16F17146 Curiosity Nano Board](https://www.microchip.com/en-us/development-tool/EV72J15A)

## Setup

![setup](images/setup.png)

PIC16F17146 Curiosity Nano board is used as development platform. External connections are not required as all the peripherals are connected internally.

## Implementation
The implementation of this example is carried out in four stages:
- Generation of Triangular wave using DAC
- Amplification of generated signal using OPA
- Sampling the OPA output using ADCC (in either single ended or differential mode)
- Printing ADCC results in graphical format using Data Visualizer

![block-diagram](images/block-diagram.png)


### Pin Connection Table

| Pin | Signal Description                    
| --- | -------------------                        
| RB5 |  EUSART RX
| RB7 |  EUSART TX         
| RC0 |  Switch SW0           
| RC1 |  LED0                 


### Generation of a Triangular Wave
The PIC16F17146 microcontroller has two DAC modules. DAC1 is used to generate a triangular wave with a frequency of 1 Hz. The frequency of the generated signal is decided by the update frequency of the  DAC register which is controlled by the Timer0 interrupt. The frequency can be modified by changing the Timer0 period. The FVR is a stable voltage reference with 1.024V, 2.048V or 4.096V selectable output levels and can be configured to supply a reference voltage to analog peripherals. The FVR2 Buffer provides a reference voltage of 1.024V to DAC2.

Frequency of the generated signal = 1/ (total number of points in one cycle * Timer0 period)`

The 8-bit DAC has 256 voltage level ranges, where each level is determined by the DAC register values that vary between 0-255. In one cycle of the signal, the DAC register updates between 0 to 255 and back to 0. The total number of points in one cycle of the triangular wave is the number of times the DAC register is updated in one cycle. In this example, the DAC register increments by four in each update, therefore the total number of points in one cycle will be 128.

### Amplification of Signal
The generated signal is amplified using OPA with a gain of two. OPA is configured in Non-Inverting Amplifier mode and the gain is set by using an internal resistor ladder feedback. The amplified signal is fed to the positive ADCC channel in both Single-Ended and Differential mode.

### ADCC Sampling
The ADCC is auto-triggered by Timer2 occurring every 2 ms. The Timer2 frequency is set ten times greater than the Timer0 frequency (Frequency of DAC update). In this example, ADCC’s Burst Average mode is used. In this mode, the ADCC module accumulates a certain number of samples (in this case, 16 samples) sequentially at a single stretch for each auto conversion trigger and computes the average of the accumulated value. This helps in reducing the signal variation present at input because of noise.

The on-board switch is used to change the ADCC mode. The switch is connected to the External Reset Source of Timer4. It will automate the switch debouncing process using monostable mode of operation of Timer4.

In Single-Ended mode of ADCC, the negative ADCC channel is connected to VSS. The input at ADCC positive channel is then converted to a digital signal. In Differential mode, DAC2 output is internally connected to a negative ADCC channel providing a constant voltage of 1.024V as shown in the below figure. FVR1 Buffer provides a reference voltage of 2.048V to ADCC. The ADCC measures the voltage difference between two inputs and converts the differential signal to digital output.


### Printing ADCC Results
The ADCC results are sent through UART and further displayed on the time plot window of the Data Visualizer. Refer to [Steps to open Time Plot window in Data Visualizer](https://github.com/microchip-pic-avr-examples/pic16f17146-curiosity-nano-demo-code-mplab-mcc#steps-to-open-time-plot-window-in-data-visualizer) for viewing results in Data Visualizer.

### Steps to Open Time Plot Window in Data Visualizer
To visualize the ADCC readings in a graphical format, time plot of Data Visualizer is used. Follow the procedure mentioned below to open Graph/Time plot in Data Visualizer.

Open the Data Visualizer tool which is available as a plugin in MPLAB X IDE.
1.	Click the Connections --> Serial Ports tab.
2.	Open COM7 Settings window by clicking on COM7 tab. Set the Baud Rate to 19200. Click Apply.

 **Note: COM port number can be different depending on the availability of port.**

3.	Click right aligned triangle symbol on COM7 tab.
4.	Click Variable Streamer.

![data-visualizer-1](images/data-visualizer-1.png)

5.	Click New Variable Streamer.
6.	Add a new variable ADFLTR of type int16_t and Click Save.

![data-visualizer-2](images/data-visualizer-2.png)

7.	Select source of the plot as COM7 port.
8.	Add plot for ADFLTR.

![data-visualizer-3](images/data-visualizer-3.png)

## Demo Operation
When the device is powered on, LED0 is turned ON. By default, ADCC is in Differential mode. The ADCC result varies between -1024 to 1023 in Differential mode.

![diff-mode](images/diff-mode.gif)

**Note : As ADCC reference voltage is 2.048V, only half ADCC range is utilized in Differential mode.**

Press the on-board switch SW0 to switch ADCC mode to Single-Ended. LED0 is turned OFF. The ADCC result varies between 0 to 4095 in Single-Ended mode.

![single-ended-mode](images/single-ended-mode.gif)

LED0 toggles on each switch press as ADCC switches between Single-Ended and Differential mode.

### Voltage Supply Configuration

Due to a target voltage other than 3.3V, the user might observe different ADCC output than mentioned above. For correct operation, set the target voltage to 3.3V. Target voltage can be set through the MPLAB® X IDE project properties, as shown below.

![target-voltage](images/target-voltage.png)

**Note : The factory default target voltage is 3.3V**

The voltage settings setup in MPLAB® X IDE is not applied immediately to the board. The new voltage setting is applied to the board when accessing the debugger, like pushing the **Refresh Debug Tool Status** button in the project dashboard tab or programming/reading program memory. Refer to [PIC16F17146 CNANO Hardware User Guide](https://www.microchip.com/DS50003388) for more details.

## Peripheral Configuration Using MCC
This section explains how to configure the peripherals using MPLAB X IDE with MCC plugin to recreate the project.

Refer to [Software Used](https://github.com/microchip-pic-avr-examples/pic16f17146-curiosity-nano-demo-code-mplab-mcc#software-used) section to install required tools to recreate the project.

Additional Links: [MCC Melody Technical Reference](https://onlinedocs.microchip.com/v2/keyword-lookup?keyword=MCC.MELODY.INTRODUCTION&redirect=true)

##### Peripheral Configuration Summary

|    Module                  |    Configuration                                                                                                                                                                                                                                                                                                                                                       |    Usage                                                                   |
|--------------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|--------------------------------------------------------------------------------|
|    Clock Control           |    Clock Source – HFINTOSC<br>   HF Internal Clock – 4 MHz<br>   Clock Divider – 1                                                                   |    System clock                                               
|    ADCC                    |    Enable ADCC<br>   Input   Configuration – Differential Mode<br>   Operating   Mode – Burst Average mode<br>   Result   Alignment – Right justified, two's compliment<br>   Positive   Reference – FVR<br>Auto-conversion Trigger – TMR2 <br>    <br>Clock Source – FOSC<br>Clock Divider – FOSC/4  <br><br>Threshold Interrupt Mode – enabled <br>Repeat – 16 <br> Accumulator Right Shift – 4   <br><br>Enable ADTI Interrupt       |    Samples OPA's output                                           |
|    DAC1                    |  Enable DAC<br>   DAC Positive Reference Selection – FVR<br>   DAC Negative Reference Selection – VSS          |    Generates Triangular Waveform                                           |
|    DAC2                    |  VDD – 3.3 <br> Required ref – 1.024<br><br>  Enable DAC<br>   DAC Positive Reference Selection – FVR  <br> DAC Negative Reference Selection – VSS        |    Connects to ADCC negative channel     
|    FVR                    |  Enable FVR <br>  FVR buffer 1 Gain – 2x <br> FVR buffer 2 Gain – 1x            |    Provides reference voltage for ADCC and DAC
|    OPA                    |    Enable Op Amp <br> Op Amp Configuration – Non-Inverting Programmable Gain Amplifier <br><br>Positive Channel – DAC1_OUT <br>Negative Channel – GSEL <br> Negative Source Selection – Vss <br><br>Internal Resistor Ladder Selection – R2/R1 = 1         |     Amplifies DAC output (Triangular Waveform)|
|    TMR0           |    Disable Timer<br>   Clock Prescaler  – 1:128<br> Timer Mode - 8bit<br> Clock Source – FOSC/4<br>Enable   Synchronisation<br>        Requested   Period – 0.0078 s s<br><br>   Enable TMR Interrupt|    Used to update DAC1                                |
|    TMR2                   |    Disable Timer<br>Control Mode – Roll over pulse <br>Start/Reset Option – Software control<br>  <br>Clock Source – MFINTOSC 32 KHz<br> Prescaler – 1:16 <br>Postscaler – 1:2 <br><br> Time Period – 0.002 |    Auto Triggers ADCC                                                       |
|    TMR4                   |    Enable Timer<br>Control Mode – Monostable <br>External Reset Source – T4INPPS<br>Start/Reset Option – Starts on rising edge on TMR4_ers<br> <br>Clock Source – MFINTOSC 32 KHz<br> Prescaler – 1:16 <br> Postscaler – 1:1 <br><br> Time Period – 0.1 <br><br> Enable TMR Interrupt |    Switch debouncing                                                   |
|    EUSART1                 |    *UART1 Driver*<br>Requested Baudrate –   19200 <br> UART PLIB Selector – EUSART1<br><br> *EUSART1 PLIB* <br>    Enable Redirect   STDIO to EUSART    <br>Enable   Receive<br>  Enable Transmit<br>   Enable Serial   Port                                                                                                                                                                                              |    Sends data to   Data Visualizer                                              |

**Note: The on-board debugger on the Curiosity Nano board which had a virtual serial port (CDC) is connected to EUSART on the PIC16F17146 and provides an easy way to communicate with the target application through terminal software. Refer to [PIC16F17146 CNANO Hardware User Guide](https://www.microchip.com/DS50003388) for more details**

## Summary
This example demonstrates how a 12-bit ADCC of PIC16F17146 works in Differential mode and Single-Ended mode using a Curiosity Nano board. The analog peripherals such as OPA, DAC and FVR can be internally connected, thus eliminating the need of external connections.
