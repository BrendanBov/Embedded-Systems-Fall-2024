//Use g++ -std=c++11 -o Lab2EX2 Lab2EX2.cpp -lwiringPi

#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <chrono>
#include <cmath>

using namespace std::chrono;
using namespace std;

// functions
void sigroutine(int);
int adcVal();
void PID(float, float, float);
float read_potentionmeter();
float read_sonar();

// variables
float distance_previous_error, distance_error;
float cummulative_error = 0.0f;
float obj_value, measured_value; // potentionmeter reading, sonar reading
int adc;
float PID_p,  PID_d, PID_total, PID_i = 0;
int time_inter_ms = 23; // time interval, you can use different time interval

/*set your pin numbers and pid values*/
int motor_pin = 26;
int sonar_pin = 1;
float kp = 20; 
float ki = 5; 
float kd = 10;

//sonar variables
const int tRising = 2;
const int tOut = 5;
const int tHoldoff = 750;
const int tInMax = 18500;

//potentiometer values
const int adcMax = 1735;
const float objMin = 10;
const float objMax = 90;
float rate_error = 0;

int main(){
	wiringPiSetup();
    adc = wiringPiI2CSetup(0x48);

    /*Set the pinMode (fan pin)*/
    pinMode(motor_pin, PWM_OUTPUT);

    // This part is to set a system timer, the function "sigroutine" will be triggered  
    // every time_inter_ms milliseconds. 
    struct itimerval value, ovalue;
    signal(SIGALRM, sigroutine);
    value.it_value.tv_sec = 0;
    value.it_value.tv_usec = time_inter_ms*1000;
    value.it_interval.tv_sec = 0;
    value.it_interval.tv_usec = time_inter_ms*1000;
    setitimer(ITIMER_REAL, &value, &ovalue);    

	while(true){
        cout<<"obj_value: "<<obj_value<<" measured_value: "<<measured_value<<endl;
        cout<<"PID_p: "<<PID_p<<endl;
        cout<<"PID_i: "<<PID_i<<endl;
        cout<<"PID_d: "<<PID_d<<endl;
        cout<<"PID_total: "<<PID_total<<endl;
        delay(100);
	}
}


void sigroutine(int signo){
    PID(kp, ki, kd);
    return;
}


/* based on the obj distance and measured distance, implement a PID control algorithm to 
the speed of the fan so that the Ping-Pang ball can stay in the obj position*/
void PID(float kp, float ki, float kd){
    /*read the objective position/distance of the ball*/
    obj_value = read_potentionmeter();

    /*read the measured position/distance of the ball*/
    measured_value = read_sonar();

    /*calculate the distance error between the obj and measured distance */
    distance_error = (obj_value - measured_value) / 100.0f;   //meters 
    cummulative_error += distance_error * (time_inter_ms / 1000.0f);   
    rate_error = (distance_error - distance_previous_error) / (time_inter_ms / 1000.0f);

    /*calculate the proportional, integral and derivative output */
    PID_p = distance_error * kp;
    PID_i = cummulative_error * ki;
    PID_d = rate_error * kd;
    PID_total = PID_p + PID_d + PID_i; 

    /*assign distance_error to distance_previous_error*/
    distance_previous_error = distance_error;

    /*use PID_total to control your fan*/
    pwmWrite(motor_pin, (int)PID_total);
    //pwmWrite(motor_pin, 915);
}


/* use a sonar sensor to measure the position of the Ping-Pang ball. you may reuse
your code in EX1.*/
float read_sonar()
{
    pinMode(sonar_pin, OUTPUT);

    digitalWrite(sonar_pin, LOW);
    usleep(tRising);

    digitalWrite(sonar_pin, HIGH);
    usleep(tOut);

    digitalWrite(sonar_pin, LOW);

    usleep(tHoldoff);

    pinMode(sonar_pin, INPUT);
    
    /*Get the time it takes for signal to leave sensor and come back.*/
    auto t1 = high_resolution_clock::now();
    int pulseWidth = 0;

    while (digitalRead(sonar_pin))
    {
        auto t2 = high_resolution_clock::now();
        pulseWidth = duration_cast<microseconds>(t2 - t1).count();
        if (pulseWidth >= tInMax) break;
    }

    /*Calculate the distance by using the time duration that you just obtained.*/ //Speed of sound is 340m/s
    float tOneWaySeconds = (pulseWidth / 2.0f) * powf(10.0f, -6.0f);
    float distanceM = 340.0f * tOneWaySeconds;
    float distanceCm = distanceM * 100.0f;
    
    return distanceCm;
}

/* use a potentiometer to set an objective position (10 - 90 cm) of the Ping-Pang ball, varying the potentiometer
can change the objective distance. you may reuse your code in Lab 1.*/
float read_potentionmeter()
{
    int digVoltage = adcVal();
    float normalizedVoltage = (float)digVoltage / adcMax;   //0.0 to 1.0
    float objPos = (objMax - objMin) * (normalizedVoltage) + objMin;

    return objPos;
}

int adcVal()
{
	uint16_t low, high, value;
	wiringPiI2CWriteReg16(adc, 0x01, 0xC5C1);
	usleep(1000);
    uint16_t data = wiringPiI2CReadReg16(adc, 0x00);


    low = (data & 0xFF00) >> 8;
    high = (data & 0x00FF) << 8;
    value = (high | low)>> 4;
	return value;
}