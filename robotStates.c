//#include <stdlib.h>
#include <avr/io.h>
//#include <avr/iox128a1.h>
#include <avr/interrupt.h>
#include <avr/AVRX_Clocks.h>
#include <avr/AVRX_Serial.h>
#include "robotStates.h"

extern int timeOutFlag;
extern int sonarFlag;
extern int stopRotateFlag;


//**************************************************************************************************
// PWMPORT = PORTE pins 0,1
// PWMTIMER = TCE0
// MOTORDIR = PORTE pins 2,3,4,5
// TIMERSONAR = TCC0
// SONARENABLE = PORTC pin 3
// SONAROUTPORT = PORTC pin 2

//in this state robot is either moving forward, sonar is active
//**************************************************************************************************
returnPackage movingState()
{

	PORTH_OUT= 0x02;

	returnPackage localStateVar;

	int haltFlag= 0;

	setupMotors();
	enableSonar();

	//set RTC to count roughly 2 seconds
	setRTC(5000);


    /*
	RTC_PER= 2000; // should be roughly x milliseconds

	CLK_RTCCTRL=0b00000101;

	RTC_INTCTRL= 0x02; //set overflow interrupt priority to med

	RTC_CTRL= 0x01; //set clock prescaler to one

	RTC_CNT= 0;
    */

	// motors A, B will have full duty cycle
    PWMTIMER_CC1= 10000;
    PWMTIMER_CC2= 10000;


	do
	{
		haltFlag= sonarFlag | timeOutFlag;

	}while(haltFlag == 0);

	RTC_CTRL= 0x00;
    RTC_CNT= 0;


	switch(haltFlag)
	{
		case 1:
			// break
			//MOTORDIR_OUT |= 0b00111100; // 0x3C
			MOTORDIR_OUT &= STOPMOVING; //0xC3

			//go to rotate left until no obstacle
			localStateVar.nextState= 2;
			localStateVar.direction= 'L';
			localStateVar.rotateQuantity= 0;
			break;
		case 2:
		case 3:
			// break
			MOTORDIR_OUT &= STOPMOVING; //0xC3

			//go to rotate right x degrees
			localStateVar.nextState= 2;
			localStateVar.direction= 'R';
			localStateVar.rotateQuantity= 500;
			break;
		default:
			break;
	}

	localStateVar.prevState= 3;
	SONAR1ENABLE_OUT &= 0b11110111;
    return localStateVar;
}



//**************************************************************************************************
//robot is rotating  until haltFlag does not equal 2
//**************************************************************************************************
returnPackage rotateState(returnPackage localStateVar)
{
    // while it is decided how to determine direction in main, this switch will stand in
    /*
    switch(haltFlag)
    {
        case 1:
            direction= 'R';
            break;
        case 2:
            direction= 'L';
            break;
        default:
            break;
    }
    */

	int rotateFlag= 1;

	setupMotors();
	enableSonar();

    switch(localStateVar.direction)
    {
        case 'l':
        case 'L':
            // if 0b11000011 is break mode, test for rotate left by forcing bits 3,4 low
            MOTORDIR_OUT |= ROTATELEFT_OR;
            MOTORDIR_OUT &= ROTATELEFT_AND;
            break;
        case 'r':
        case 'R':
            MOTORDIR_OUT |= ROTATERIGHT_OR;
            MOTORDIR_OUT &= ROTATERIGHT_AND;
            //set rtc for .3 seconds
            setRTC(localStateVar.rotateQuantity);
            break;
        default:
            break;
    }

    // motors A, B will have full duty cycle
    PWMTIMER_CC1= 10000;
    PWMTIMER_CC2= 10000;


	do
	{
		rotateFlag= sonarFlag | stopRotateFlag;

	}while(rotateFlag == 1);

	RTC_CTRL= 0x00;
    RTC_CNT= 0;

	// test for break mode break mode
	MOTORDIR_OUT &= STOPMOVING; // 0x3C

    switch(stopRotateFlag)
    {
        case 0:
            localStateVar.nextState= 3;
            break;
        case 2:
            localStateVar.nextState= 1;
            break;
        default:
            break;
    }
	localStateVar.direction= 'z';
	localStateVar.rotateQuantity= 0;
	return localStateVar;

}



//**************************************************************************************************
// PWMPORT = PORTE pins 0,1
// PWMTIMER = TCE0
// MOTORDIR = PORTE pins 2,3,4,5
// TIMERSONAR = TCC1
// SONARENABLE = PORTC pin 3
// SONAROUTPORT = PORTC pin 2
//**************************************************************************************************
void setupMotors()
{
	PWMPORT_DIR |= 0b00001100; //set port direction to output
	//PWMPORT_OUT= 0;

	PWMTIMER_CTRLA =  0x05; //set prescaler to clk/64
	PWMTIMER_CTRLB = 0xC3; //turn on capture C, and D and set waveform generation mode to single slope PWM
	PWMTIMER_CTRLD = 0x00; //turn off events
	PWMTIMER_CTRLE = 0x00; //turn off byte mode
	PWMTIMER_PER = 10000; //set the top of the period so we have a total period of 20ms(for servo)

	MOTORDIR_DIR |= 0b11110000; //0xF0
	// force bits 2,6 low
	MOTORDIR_OUT &= MOVEFORWARD_AND; //0xAF
	// force bits 0,4(motor direction) high
	MOTORDIR_OUT |= MOVEFORWARD_OR; //0xA0
}



//**************************************************************************************************
// PWMPORT = PORTE pins 0,1
// PWMTIMER = TCE0
// MOTORDIR = PORTE pins 2,3,4,5
// TIMERSONAR = TCC1
// SONARENABLE = PORTC pin 3
// SONAROUTPORT = PORTC pin 2
//**************************************************************************************************
void enableSonar()
{
	SONAR1OUTPORT_DIR &= 0b11111110; //0xFE
	SONAR1OUTPORT_PINCTRL= 0x00; // set input sense to both edges

	SONAR2OUTPORT_DIR &= 0b11101111; //0xFB
	SONAR2OUTPORT_PINCTRL= 0x00; // set input sense to both edges

	EVSYS_CH1MUX= 0x78;//0b01100010 set event channel 1 to portf pin 0
	EVSYS_CH3MUX= 0x7C;//0b01100010 set event channel 1 to port7 pin 4

	EVSYS_CH1CTRL=0x00; // set event channel 1 to no sample filtering
	EVSYS_CH3CTRL=0x00; // set event channel 3 to no sample filtering

	TIMERSONAR1_CTRLA = 0x05; //set clock source sysclk/64= 500KHz
	TIMERSONAR1_CTRLB = 0x10; //turn on capture channel A and set waveform generation mode normal
	TIMERSONAR1_CTRLD = 0xC9; //set events to Pulse Width capture, no timer delay, and listen to event channel 1
	TIMERSONAR1_CTRLE = 0x00; //turn off byte mode
	TIMERSONAR1_PER = 0xFFFF; //set the top of the period to max 16-bit value

	TIMERSONAR1_INTCTRLA= 0x00; // turn other interrupts off
	TIMERSONAR1_INTCTRLB= 0x03; //set capture interrupt to high


	TIMERSONAR2_CTRLA = 0x05; //set clock source sysclk/64= 500KHz
	TIMERSONAR2_CTRLB = 0x10; //turn on capture channel A and set waveform generation mode normal
	TIMERSONAR2_CTRLD = 0xCB; //set events to Pulse Width capture, no timer delay, and listen to event channel 1
	TIMERSONAR2_CTRLE = 0x00; //turn off byte mode
	TIMERSONAR2_PER = 0xFFFF; //set the top of the period to max 16-bit value

	// set pin 1 direction to output
	SONAR1ENABLE_DIR |= 0b00000010; // 0x08;

	// set pin 3 direction to output
	SONAR2ENABLE_DIR |= 0b00001000; // 0x08;

	// set pin 1 high
	SONAR1ENABLE_OUT |= 0b00000010; // 0x08;

	// set pin 3 high
	SONAR2ENABLE_OUT |= 0b00001000; // 0x08;

}

//**************************************************************************************************
// PWMPORT = PORTE pins 0,1
// PWMTIMER = TCE0
// MOTORDIR = PORTE pins 2,3,4,5
// TIMERSONAR = TCC1
// SONARENABLE = PORTC pin 3
// SONAROUTPORT = PORTC pin 2
//**************************************************************************************************
void setRTC(int topValue)
{
	RTC_PER= topValue; // should be roughly x milliseconds

	CLK_RTCCTRL=0b00000101;

	RTC_INTCTRL= 0x02; //set overflow interrupt priority to med

	RTC_CTRL= 0x01; //set clock prescaler to one

	RTC_CNT= 0;
}