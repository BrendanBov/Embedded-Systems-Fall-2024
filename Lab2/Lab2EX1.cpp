//Use g++ -std=c++11 -o Lab2EX1 Lab2EX1.cpp -lwiringPi

#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <wiringPi.h>
#include <ctime>
#include <ratio>
#include <chrono>
#include <math.h>
using namespace std;
using namespace std::chrono;
using namespace std::chrono;

const int depthPin = 1;
const int tRising = 2;
const int tOut = 5;
const int tHoldoff = 750;
const int tInMax = 18500;

int main(){
	//Set up wiringPi
	wiringPiSetup();
	
	while (true)
	{
		/*Set the pinMode to output and generate a LOW-HIGH-LOW signal using "digitalWrite" to trigger the sensor. 
		Use a 2 us delay between a LOW-HIGH and then a 5 us delay between HIGH-LOW. You can use
		the function "usleep" to set the delay. The unit of usleep is microsecond. */
		pinMode(depthPin, OUTPUT);

		digitalWrite(depthPin, LOW);
		usleep(tRising);

		digitalWrite(depthPin, HIGH);
		usleep(tOut);
	
		digitalWrite(depthPin, LOW);

		/*Echo holdoff delay 750 us*/
		usleep(tHoldoff);

		/*Switch the pinMode to input*/ 
		pinMode(depthPin, INPUT);
		
		/*Get the time it takes for signal to leave sensor and come back.*/

		// 1. defind a varable to get the current time t1. Refer to "High_Resolution_Clock_Reference.pdf" for more information
		auto t1 = high_resolution_clock::now();
		int pulseWidth = 0;
		/*read signal pin, the stay is the loop when the signal pin is high*/ 
		while (digitalRead(depthPin))
		{
			// 2. defind a varable to get the current time t2.
			auto t2 = high_resolution_clock::now();
			// 3. calculate the time duration: t2 - t1
			pulseWidth = duration_cast<microseconds>(t2 - t1).count();
			// 4. if the duration is larger than the Pulse Maxium 18.5ms, break the loop.
			if (pulseWidth >= tInMax) 
			{
				//cout << "break";
				break;
			}
		}

		//cout << pulseWidth;

		/*Calculate the distance by using the time duration that you just obtained.*/ //Speed of sound is 340m/s
		float tOneWaySeconds = (pulseWidth / 2.0f) * powf(10.0f, -6.0f);
		float distanceM = 340.0f * tOneWaySeconds;
		float distanceCm = distanceM * 100.0f;

		/*Print the distance.*/
		cout << "Distance (cm): " << distanceCm << '\n';

		/*Delay before next measurement. The actual delay may be a little longer than what is shown is the datasheet.*/
		usleep(200);
	}
}
