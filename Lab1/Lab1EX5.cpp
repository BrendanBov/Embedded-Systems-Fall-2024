// g++ -std=c++11 -o Lab1EX5 Lab1EX5.cpp -lwiringPi

#include <wiringPi.h>
#include <softPwm.h>
#include <stdio.h>
#include <wiringPiI2C.h>
#include <string.h>
#include <iostream>
#include <softPwm.h>
#include <math.h>
#include <stdlib.h>
#include <ctime>
#include <signal.h>
#include <iomanip>
#include <unistd.h>


#define SERVO_MIN_MS 5
#define SERVO_MAX_MS 25
#define SERVO_MIN_ANGLE 0
#define SERVO_MAX_ANGLE 180
using namespace std;

/* signal pin of the servo*/
#define servoPin 1
#define buttonPin 4 
#define adcMax 1735 

bool clockwise = true;

long tempTime = 0;

int adcVal();

//Specific a certain rotation angle (0-180) for the servo
void servoWrite(int pin, int angle)
{ 
    //long time = 5 + (angle / 90.0) * 10;
    long time = 5 + (int)(angle * 10 / 90.0);
    tempTime = time;
    //cout << " time: " << time << endl;
    softPwmWrite(pin, time);    
}

/* Sefind your callback function to handout the pressing button interrupts. */
void press_button()
{
    clockwise = !clockwise;
}

int main(void)
{
    wiringPiSetup();    
    softPwmCreate(servoPin,  0, 200);

    /* Use wiringPiISR() to setup your interrupts. Refer to document WiringPi_ Interrupts.pdf. */
    wiringPiISR(buttonPin, INT_EDGE_RISING, &press_button);

    while(1)
    {
        /* read ADS1015 value */
        int digVoltage = adcVal();
        /* convert the obtained ADS1015 value to angle 0 - 180*/
        //TODO need to find potentiometer max
        int angle = (int)(180 * ((float)digVoltage / adcMax));
        //int angle = 180 * digVoltage/1735.0;
        if(!clockwise) angle = 180 - angle;
        cout << "voltage: " << digVoltage << " angle: " << angle << " is clockwise: " << clockwise << " time(ms): " << tempTime << '\n';

        /* use the angle to control the servo motor*/
        servoWrite(servoPin, angle);

        usleep(100000);

    }
    return 0;
}

//This function is used to read data from ADS1015
int adcVal()
{
	uint16_t low, high, value;
	// Refer to the supplemental documents to find the parameters. In this lab, the ADS1015
	// needs to be set in single conversion, single-end mode, FSR (full-scale range)is 6.144, you can choose 
	// any input pin (A0, A1, A2, A3) you like.
	int adc = wiringPiI2CSetup(0x48);
	wiringPiI2CWriteReg16(adc, 0x01, 0xC5C1);
	usleep(1000);
    uint16_t data = wiringPiI2CReadReg16(adc, 0x00);


    low = (data & 0xFF00) >> 8;
    high = (data & 0x00FF) << 8;
    value = (high | low)>> 4;
	return value;
}