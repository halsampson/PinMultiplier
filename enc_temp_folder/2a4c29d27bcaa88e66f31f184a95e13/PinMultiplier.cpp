// PinMultiplier.cpp

/*
Use SN74HC595 8 bit shift register as a pin multiplier

Only one output pin is used, feeding SRCLK, RCLK, and SER (data) through a filter resistor
*/

#define COM_PORT "COM7" 

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <time.h>
#include <stdio.h>
#include <conio.h>

HANDLE hCom;

void openSerial() {
	hCom = CreateFileA("\\\\.\\" COM_PORT, GENERIC_WRITE | GENERIC_READ, 0, NULL, OPEN_EXISTING, NULL, NULL);
	if (hCom == INVALID_HANDLE_VALUE) exit(-2);

	DCB dcb = { 0 };
	dcb.DCBlength = sizeof(DCB);
	dcb.BaudRate = 2000000;  // CDC USB
	dcb.ByteSize = 8;
	dcb.fBinary = TRUE;
	if (!SetCommState(hCom, &dcb)) exit(-3);
}

void setBits(unsigned char bits) {
	if (!hCom) openSerial();

	// QA = MSB; QH = LSB (sent first)
  // All positive edges (0 --> 1 or Stop bit) clock in previous, filtered data
	// Extra clock transfers from shift register to output latch

#if 0	// 5:1 timing ratio - faster
	// Want 5:1 tau ~ sqrt(5) / baud = 1.1us / (3 pF typ. + 1pF PCB capacitance) ~ 270 K Ohms
  const unsigned char Equalize = 0x55;
	WriteFile(hCom, &Equalize, 1, NULL, NULL);
	const unsigned char BitChar[2] = { 0x0F, 0xFF };   
		for (int bit = 0; bit <= 8; bit++) {
		WriteFile(hCom, &BitChar[bits & 1], 1, NULL, NULL);
		bits >>= 1;
	}
#else // 9:1 margin by inserting a delay between 0 and 1 -- more tolerant of filter variation
	// Want 9:1 tau ~ sqrt(9) / baud = 1.5us / (3 pF typ. + 1 pF PCB capacitance) ~ 390 K Ohms
	const unsigned char BitChar[2] = { 0x00, 0xFF }; 
	int lastBit = 0;
	for (int bit = 0; bit <= 8; bit++) {
    int thisBit = bits & 1;
    if (thisBit && !lastBit) Sleep(0);  // short delay to allow the RC filter to charge up
		WriteFile(hCom, &BitChar[thisBit], 1, NULL, NULL);
		bits >>= 1;
	}
#endif
}

int main() {
	while (1) {
		setBits(0xAA);
		Sleep(20);
		setBits(0x55);
		Sleep(10);
	}

  return 0;
}
