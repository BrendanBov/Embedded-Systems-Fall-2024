//Use g++ joystick.cc -std=c++11 -o Lab3EX2 Lab3EX2.cpp -lwiringPi
// Make sure controller is on 'X' settings
#include <iostream>
#include <wiringPi.h>
#include <wiringSerial.h>
#include "joystick.hh"
#include <unistd.h>
#include <cstdlib>
using namespace std;

void movement(int, int);
void rotateSequence(bool);
void translateSequence(bool);
bool stopMovement();
int kobuki;

Joystick joystick("/dev/input/js0");
JoystickEvent event;

//kubuki motor control constants
const int baud = 20;
const int r = 500;
const int bias = 230;
const int speed = 200;
const float lostPackageMod = 1.4f;

const int dPadXAxis = 6;
const int dPadYAxis = 7; 
const int startBtn = 7;
const int selectBtn = 8;

bool runFlag = true;

int main(){

	wiringPiSetup();
	kobuki = serialOpen("/dev/kobuki", 115200);
	unsigned int button;

	//The joystick creates events when a button or axis changes value.
	//Sample event from the joystick: joystick.sample(&event)

	//You can interpret these by sampling the events.
	//Each event has three parameters.
	//A type, axis or button,
	//judge if the event is button: event.isButton()
	//judge if the event is axis: event.isAxis()
	//A number corresponding to the axis or button pressed: event.number
	//And a value, Buttons: 0-unpressed, 1-pressed, Axis: -32767 to 0 to 32767: event.value

	while(runFlag){
		/*Create a series of commands to interpret the
		joystick input and use that input to move the Kobuki*/

		//Use the following Key Map:
		//Up     - move the Kobuki forward
		//Down   - move the Kobuki backward
		//Left   - rotate the Kobuki 90 degrees counterclockwise
		//Right  - rotate the Kobuki 90 degrees clockwise
		//Start  - immediately stop the Kobuki's movement
		//Select - exit the script and close the Kobuki's connection cleanly
		if (joystick.sample(&event))
		{
			if (event.isButton())
			{
				printf("isButton: %u | Value: %d\n", event.number, event.value);
				/*Interpret the joystick input and use that input to move the Kobuki*/
				if(stopMovement()) break;
			}
			if (event.isAxis())
			{
				printf("isAxis: %u | Value: %d\n", event.number, event.value);
				/*Interpret the joystick input and use that input to move the Kobuki*/
				if (event.number == dPadXAxis)
					rotateSequence(event.value > 0);	// > 0, clockwise
				else if (event.number == dPadYAxis)
					translateSequence(event.value < 0);
			}
		}
		else
		{
			movement(0,0);
		}
	}

	return 0;
}

void rotateSequence(bool clockwise)
{
	int rotMod = clockwise ? 1 : -1;
	float radSpeed = (3.14159f / 2) / 1;	// pi/2rad / 1second
	int rotSpeed = -(int)(radSpeed * (bias / 2));	// w * b / 2
	rotSpeed *= rotMod * lostPackageMod;
	/*Rotate the Kobuki 90 degrees*/
	for (int i = 0; i < 1000 / baud; i++) 
	{
		joystick.sample(&event);
		if (stopMovement()) return;
		movement(rotSpeed, 1);
	}
	movement(0,0);
}

void translateSequence(bool fwd)
{
	int speedMod = fwd ? 1 : -1;
	int translationSpeed = (int)(speed * speedMod * lostPackageMod);
	while(runFlag)
	{
		if (joystick.sample(&event) && event.isAxis() && event.number == dPadYAxis)
			if (event.value == 0) return;
		if (stopMovement()) return;
		movement(translationSpeed, 0);
	}
	movement(0,0);
}

bool stopMovement()
{

	if (event.isButton())	//may cause stop on release and start
	{
		if (event.number == startBtn && event.value == 1) return true;
		if (event.number == selectBtn && event.value == 1) 
		{
			movement(0,0);
			serialClose(kobuki);
			runFlag = false;
			return true;
		}
	}
	return false;
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
