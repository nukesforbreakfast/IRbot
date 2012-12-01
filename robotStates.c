//#include <stdlib.h>
#include <avr/io.h>
#include <avr/iox128a1.h>
#include <avr/interrupt.h>
#include <avr/AVRX_Clocks.h>
#include <avr/AVRX_Serial.h>
#include "robotStates.h"

// PWMPORT = PORTC pins 0,1
// PWMTIMER = TCC0
// MOTORDIR = PORTE pins 0,2,4,6
// TIMERSONAR = TCC1
// SONARENABLE = PORTC pin 3
// SONAROUTPORT = PORTC pin 2


extern volatile int haltFlag;

//in this state robot is stationary, servo is active, as is IR receiver
int searchingState()
{
	int localStateVar=1;

	PORTC_DIR = 0xFF; //set port direction to output
	PORTC_OUT= 0;

	TCC0_CTRLA =  0x05; //set prescaler to clk/64
	TCC0_CTRLB = 0x73; //turn on capture A,B,C and set waveform generation mode to single slope PWM
	TCC0_CTRLD = 0x00; //turn off events
	TCC0_CTRLE = 0x00; //turn off byte mode
	TCC0_PER = 10000; //set the top of the period so we have a total period of 20ms(for servo)

	//Motor A
	TCC0_CCA = 0; //set the compare register value to achieve a 0% duty cycle
	//Motor B
	TCC0_CCB = 0; //set the compare register value to achieve a 0% duty cycle
	//scanner servo
	TCC0_CCC = 300; //set the compare register value to achieve a 1ms on time at start. range is from 500-1000

	localStateVar= 2;
    return localStateVar;
}


// PWMPORT = PORTC pins 0,1
// PWMTIMER = TCC0
// MOTORDIR = PORTE pins 0,2,4,6
// TIMERSONAR = TCC1
// SONARENABLE = PORTC pin 3
// SONAROUTPORT = PORTC pin 2

//in this state robot is either moving forward or rotating left, sonar is active
int movingState()
{
	int localStateVar=2;

	//setupMotors();




	//enableSonar();

	SONAROUTPORT_DIR &= 0b11111011; //0xFB
	SONAROUTPORT_PIN2CTRL= 0x00; // set input sense to both edges

	EVSYS_CH1MUX= 0x62;//0b01100010 set event channel 1 to portc pin 2
	EVSYS_CH1CTRL=0x00; // set event channel 1 to no sample filtering

	TIMERSONAR_CTRLA = 0x05; //set clock source sysclk/64= 500KHz
	TIMERSONAR_CTRLB = 0x10; //turn on capture channel A and set waveform generation mode normal
	TIMERSONAR_CTRLD = 0xC9; //set events to Pulse Width capture, no timer delay, and listen to event channel 1
	TIMERSONAR_CTRLE = 0x00; //turn off byte mode
	TIMERSONAR_PER = 0xFFFF; //set the top of the period to max 16-bit value

	TIMERSONAR_INTCTRLA= 0x00; // turn other interrupts off
	TIMERSONAR_INTCTRLB= 0x03; //set capture interrupt to high

	// set pin 3 direction to output
	SONARENABLE_DIR |= 0b00001000; // 0x08;

	// set pin 3 high
	SONARENABLE_OUT |= 0b00001000; // 0x08;





	haltFlag= 0;
	PORTH_OUT= 0x02;

	// motors A, B will have full duty cycle
    PWMTIMER_CCA= 10000;
    PWMTIMER_CCB= 10000;

	haltFlag=0;
	while(haltFlag == 0)
	{
		;
	}
    if(haltFlag == 1)
    {
        // test for freerun mode
        MOTORDIR_OUT &= 0b10101010; //0xAA
        //MOTORDIR_OUT |= 0b01010101; // 0x55

        //move to searching state
        localStateVar= 1;
    }
    /*
    else if(haltFlag == 2)
    {
        // test for break mode break mode
        MOTORDIR_OUT |= 0b01010101; // 0x55
        rotateRobot();

		//stay in moving state
		localStateVar= 2;
    }
    */
	PORTH_OUT= 0;
    return localStateVar;
}

void rotateRobot()
{
	int prevRTC_PER= RTC_PER;

	RTC_PER= 1000;

	// if 0b01010101 is break mode, test for rotate left by forcing bits 2,4 low
	MOTORDIR_OUT &= 0b11101011;

	RTC_CNT=0;

	haltFlag= 0;
	while(haltFlag == 0)
	{
		;
	}
	// test for break mode break mode
	MOTORDIR_OUT |= 0b01010101; // 0x55

	//return RTC_PER to its previous value
	RTC_PER= prevRTC_PER;
}


// PWMPORT = PORTC pins 0,1
// PWMTIMER = TCC0
// MOTORDIR = PORTE pins 0,2,4,6
// TIMERSONAR = TCC1
// SONARENABLE = PORTC pin 3
// SONAROUTPORT = PORTC pin 2
void setupMotors()
{
	PWMPORT_DIR |= 0b00000011; //set port direction to output
	//PWMPORT_OUT= 0;

	PWMTIMER_CTRLA =  0x05; //set prescaler to clk/64
	PWMTIMER_CTRLB = 0x33; //turn on capture A,B and set waveform generation mode to single slope PWM
	PWMTIMER_CTRLD = 0x00; //turn off events
	PWMTIMER_CTRLE = 0x00; //turn off byte mode
	PWMTIMER_PER = 10000; //set the top of the period so we have a total period of 20ms(for servo)

    MOTORDIR_DIR= 0b01010101; //0x55
    // force bits 0,4(motor direction) high
    MOTORDIR_OUT |= 0b00010001; //0x11
    // force bits 2,6 low
    MOTORDIR_OUT &= 0b10111011; //0xBB

	CLK_RTCCTRL=0x0B;
    RTC_CTRL= 0x04; //set clock prescaler to 1/16

    RTC_INTCTRL= 0x00; //set interrupt priority to OFF!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    RTC_PER= 8000; // should be roughly 4 seconds
}


// PWMPORT = PORTC pins 0,1
// PWMTIMER = TCC0
// MOTORDIR = PORTE pins 0,2,4,6
// TIMERSONAR = TCC1
// SONARENABLE = PORTC pin 3
// SONAROUTPORT = PORTC pin 2
void enableSonar()
{
	SONAROUTPORT_DIR &= 0b11111011; //0xFB
	SONAROUTPORT_PIN2CTRL= 0x00; // set input sense to both edges

	EVSYS_CH1MUX= 0x62;//0b01100010 set event channel 1 to portc pin 2
	EVSYS_CH1CTRL=0x00; // set event channel 1 to no sample filtering

	TIMERSONAR_CTRLA = 0x05; //set clock source sysclk/64= 500KHz
	TIMERSONAR_CTRLB = 0x10; //turn on capture channel A and set waveform generation mode normal
	TIMERSONAR_CTRLD = 0xC9; //set events to Pulse Width capture, no timer delay, and listen to event channel 1
	TIMERSONAR_CTRLE = 0x00; //turn off byte mode
	TIMERSONAR_PER = 0xFFFF; //set the top of the period to max 16-bit value

	TIMERSONAR_INTCTRLA= 0x00; // turn other interrupts off
	TIMERSONAR_INTCTRLB= 0x03; //set capture interrupt to high

	// set pin 3 direction to output
	SONARENABLE_DIR |= 0b00001000; // 0x08;

	// set pin 3 high
	SONARENABLE_OUT |= 0b00001000; // 0x08;
}
