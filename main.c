/*
Mikko Uuksulainen, 2012
mikko.uuksulainen@gmail.com
Description: Code for controlling MSP430 based sensor guided car
*/

#include "msp430G2231.h"
#include "stdio.h"

// Motor control
const char leftMotorOn = 0x02;
const char rightMotorOn = 0x04;
const char bothMotorsOn = 0x06;

// Sensor values
const char leftSensorBlack = 0x10;
const char middleSensorBlack = 0x20;
const char rightSensorBlack = 0x80;
const char allSensorsBlack = 0xB0;

const char randomIns = 0x0E;

char waitModeActivated;
enum STATE {learning, driving, drivingFromMemory};
enum STATE state;

// Functions
char doDriving();
void doLearning(char lastMove);

main() 
{
	// Kill Watchdog Timer
	WDTCTL = WDTPW + WDTHOLD;
	  
	CCTL0 = CCIE;                             // CCR0 keskeytykset hyväksytään (TACCTL0)
	TACTL = TASSEL_2 + MC_1 + ID_3;           // SMCLK/8, upmode 
	CCR0 =  10000;                             // 125 Hz   
	
	P1IE = 0x08; // TAI "P1IE |= BIT3;" | Hyväksyy keskeytykset portista P1.3
	P1IFG = 0x00; //TAI "P1IFG &= ~BIT3" | Pyyhkii keskeytysliput (asettaa nollille, jos on 1, suoritetaan keskeytys)
	
	P1REN = 0x0E;
	P1DIR = 0x00;
	P1OUT = 0x00;
	
	__bis_SR_register(GIE); // Statusrekisteristä Global Interrupt Enable lippu ylös, hyväksyy kaikki keskeytykset
	
	waitModeActivated = 0;
	state = driving;
	//memoryPositionCounter = 0;
	
	while(1)
	{
	}
}

// Portin 1 keskeytyspalvelu
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
  	if (waitModeActivated == 1) {
		puts("Wait Mode Deactivated");
		waitModeActivated = 0;
  	} else {
  		waitModeActivated = 1;
		puts("Wait Mode Activated");
  	}
	P1IFG = 0x00; // Pyyhitään keskeytysliput
}

// Timer A0 keskeytyspalvelu
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A (void)
{
  	if (waitModeActivated == 0) {
	  switch(state) {
	  	case driving:
		  doDriving();
		  break;
		case learning:
		  doLearning(doDriving());
		  break;
		case drivingFromMemory:
		  puts("drivingFromMemory");
		  break;
	  }
	}
}

// Returns last move
char doDriving() {
	
	puts("driving");
	
	// If driving black road
	if (P1IN == allSensorsBlack + randomIns || P1IN == allSensorsBlack + bothMotorsOn + randomIns) {
		P1DIR = bothMotorsOn;
		P1OUT = bothMotorsOn;
		return bothMotorsOn;
	}
	
	// If driving thin black road
	if (P1IN == middleSensorBlack + randomIns || P1IN == middleSensorBlack + bothMotorsOn + randomIns) {
		P1DIR = bothMotorsOn;
		P1OUT = bothMotorsOn;
		return bothMotorsOn;
	}
	
	// If driving in white
	if (P1IN == randomIns || P1IN == bothMotorsOn + randomIns) {
		P1DIR = 0x00;
		P1OUT = 0x00;
		return 0x00;
	}
	
	// Too much on the right
	if (P1IN == leftSensorBlack + randomIns || P1IN == leftSensorBlack + randomIns + bothMotorsOn) {
		P1DIR = rightMotorOn;
		P1OUT = rightMotorOn;
		return rightMotorOn;
	}
	
	// Too much on the right
	if (P1IN == rightSensorBlack + randomIns || P1IN == rightSensorBlack + randomIns + bothMotorsOn) {
		P1DIR = leftMotorOn;
		P1OUT = leftMotorOn;
		return leftMotorOn;
	}
	return 0;
}