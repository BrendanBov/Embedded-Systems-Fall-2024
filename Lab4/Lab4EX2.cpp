//use g++ -std=c++11 -o Lab4EX2 Lab4EX2.cpp -lwiringPi


#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <wiringPi.h>
#include <wiringSerial.h>
#include <ctime>
#include <ratio>
#include <chrono>
#include <math.h>
using namespace std;
using namespace std::chrono;

int kobuki;
float read_sonar();
void rotateSequence(bool);
void moveTillWall();
void movement(int, int);

// sonar config parameters
const int sonar_pin = 1;
const int tRising = 2;
const int tOut = 5;
const int tHoldoff = 750;
const int tInMax = 18500;

// movement config parameters
const int baud = 20;
const int r = 500;
const int bias = 230;
const int speed = 200;
const float lostPackageMod = 1.4f;

// behavoir parameters
const float turnAtDistanceCm = 20.0f;
const int timeoutMS = 10000;	// 10s timout

int main(){
	wiringPiSetup();
	kobuki = serialOpen("/dev/kobuki", 115200);

	/*Move from a random point within the area designated "X" to the
	point B as shown on the diagram. Use a sonar sensor to navigate through the channel.
	You can reuse your code from Lab 2 and 3*/

	/*Note: the Kobuki must completely pass point B as shown to receive full credit*/
	moveTillWall();
	rotateSequence(true);
	moveTillWall();
	rotateSequence(false);
	moveTillWall();

	//while(true) cout << read_sonar() << endl;
}

void rotateSequence(bool clockwise)
{
	int rotMod = clockwise ? 1 : -1;
	float radSpeed = (3.14159f / 2) / 1;	// pi/2rad / 1second
	int rotSpeed = -(int)(radSpeed * (bias / 2));	// w * b / 2
	rotSpeed *= rotMod * lostPackageMod;
	/*Rotate the Kobuki 90 degrees*/
	for (int i = 0; i < 1000 / baud; i++)  movement(rotSpeed, 1);
	
	movement(0,0);
}

void moveTillWall()
{
	int translationSpeed = (int)(speed * lostPackageMod);
	for (int i = 0; i < timeoutMS / baud; i++)
	{
		float distCm = read_sonar();
		cout << read_sonar() << endl;
		if (distCm < turnAtDistanceCm) break;
		movement(translationSpeed, 0);
	}
	movement(0,0);
}


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
    
	usleep(200);
    return distanceCm;
}

void movement(int sp, int r){
	//Create the byte stream packet with the following format:
	unsigned char b_0 = 0xAA; /*Byte 0: Kobuki Header 0*/
	unsigned char b_1 = 0x55; /*Byte 1: Kobuki Header 1*/
	unsigned char b_2 = 0x06; /*Byte 2: Length of Payload*/
	unsigned char b_3 = 0x01; /*Byte 3: Sub-Payload Header*/
	unsigned char b_4 = 0x04; /*Byte 4: Length of Sub-Payload*/

	unsigned char b_5 = sp & 0xff;	//Byte 5: Payload Data: Speed(mm/s)
	unsigned char b_6 = (sp >> 8) & 0xff; //Byte 6: Payload Data: Speed(mm/s)
	unsigned char b_7 = r & 0xff;	//Byte 7: Payload Data: Radius(mm)
	unsigned char b_8 = (r >> 8) & 0xff;	//Byte 8: Payload Data: Radius(mm)
	unsigned char checksum = 0;		//Byte 9: Checksum
	
	//Checksum all of the data
	char packet[] = {b_0,b_1,b_2,b_3,b_4,b_5,b_6,b_7,b_8};
	for (unsigned int i = 2; i < 9; i++)
		checksum ^= packet[i];

	/*Send the data (Byte 0 - Byte 8 and checksum) to Kobuki using serialPutchar (kobuki, );*/
	//serialPutchar(kobuki, packet);
	for (char c : packet) serialPutchar(kobuki, c);
	serialPutchar(kobuki, checksum);
	/*Pause the script so the data send rate is the
	same as the Kobuki data receive rate*/
	delay(baud);	// data rate of 20ms
}