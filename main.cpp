/* ECG Simulator from SD Card - Receiver
 * 
 * This program uses a Nucleo F303K8 to receive a short from another
 * F303K8 and convert it to a float value from 0 - 1 for output from an analog
 * output pin. The short sent from the other Nucleo (referred to as Sender) is 
 * divided by 2048 because it was obtained via an 11-bit ADC.
 * 
 * Modified heavily from: https://forums.mbed.com/t/two-nucleo-serial-communication-via-tx-and-rx-and-vice-versa/8131
 * 
 * Authors:    Amit J. Nimunkar and Lucas N. Ratajczyk
 * Date:       05-24-2021
 * 
 * Modified by Royal Oakes, 02-02-2022.
 */

#include "mbed.h"

//Serial      pc(USBTX,USBRX);    // Optionally - Set up serial communication with the host PC for printing statement to console
Serial      sender(D1,D0);      // Set up serial communication with the Nucleo which is sending the signal to us (Sender)
AnalogOut   Aout(A3);           // Initialize an analog output pin to display the signal extracted from Sender

// This union is used to recieve data from the sender. Use data.s to access the
// data as a short, and use data.h to access the individual bytes of data.
typedef union _data {
    short s;
    char h[sizeof(short)];
} myShort;

myShort data;

char d;         // Variable to hold the current byte extracted from Sender
int num;        // Integer version of the myShort value
int i = 0;      // Index counter for number of bytes received
float samp_rate = 360.0f;           // Sample rate of the ISR

// Ticker for the ISR
Ticker sampTick; 

// Prototypes
void ISRfxn();


int main() {
    // Set up serial communication
    sender.baud(115200);
    //pc.baud(115200); // Optional debugging.
    
    // Sample num at a fixed rate
    sampTick.attach(&ISRfxn, 1.0f/samp_rate);
    
    // Get data from sender
    while (1) {
        
        // Get the current character from Sender
        d = sender.getc();
        
        // If the byte we got was a '\0', it is possibly the terminator
        if (d == '\0' && i >= sizeof(short)){
            i = 0;                          // Reset index counter.
            num = (int) data.s;             // Convert the short to an int.
        } else if (i < sizeof(short)) {     // If 2 bytes haven't been received, 
            data.h[i++] = d;                // then the byte is added to data
        }
    }
}


/* Interrupt function. Computes the ADC value of num and outputs the voltage.
 */
void ISRfxn() {
    // Convert the number we extracted from Sender into a float with scale 0 - 1 (note
    // division by 2048 due to acquisition of data by an 11-bit ADC) and output it from A3
    float fnum = (float) num/2048.0f;
    Aout = fnum;
}
