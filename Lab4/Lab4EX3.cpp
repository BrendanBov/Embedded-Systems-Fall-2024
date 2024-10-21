//Use g++ -std=c++11 -o Lab4EX3 Lab4EX3.cpp -lwiringPi

#include <string>
#include <iostream>
#include <wiringSerial.h>
#include <wiringPi.h>
#include <unistd.h>
#include <cstdlib>
#include <iomanip>
using namespace std;

int kobuki;

// sensor parameters
unsigned int bumper;
unsigned int drop;
unsigned int cliff;
unsigned int button;
unsigned int kRead; 

void movement(int, int);
void rotateSequence(bool);
void translateSequence(bool);
void readData();

void backupAndRotate(bool);

// movement config parameters
const int baud = 20;
const int r = 500;
const int bias = 230;
const int speed = 100;
const float lostPackageMod = 1.4f;

unsigned int sendRate = 20000;

// misc parameters
unsigned int backupTime = 500;

int main(){
	//Create connection to the Kobuki
	wiringPiSetup();
	kobuki = serialOpen("/dev/kobuki", 115200);

	readData();

	while(serialDataAvail(kobuki) != -1){
		/*Read the initial data. If there are no flags,
		the default condition is forward.*/
		movement(speed, 0);

		// backup and rotate if cliff is reached or bumper is hit
		if ((cliff <= 0x07) && (bumper  <= 0x07))
		{
			//right bumper / cliff
			if (((cliff & 0x01) == 0x01) || ((bumper & 0x01) == 0x01))
				backupAndRotate(false);
			//center bumper / cliff
			else if (((cliff & 0x02) == 0x02) || ((bumper & 0x02) == 0x02))
				backupAndRotate(true);
			//left bumper / cliff
			else if (((cliff & 0x04) == 0x04) || ((bumper & 0x04) == 0x04))
				backupAndRotate(true);

		}
		
		
		/*Cleanly close out of all connections using Button 1.*/
		/*Use serialFlush(kobuki) to discard all data received, or waiting to be send down the given device.*/
		if (button == 0x02) 
		{
			//serialFlush(kobuki);
			movement(0,0);
			serialClose(kobuki);
			return 0;	
		}

		//Pause the script so the data read receive rate is the same as the Kobuki send rate.
		usleep(sendRate);

		serialFlush(kobuki);
		readData();
	}
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

void readData(){
	while(true){
		//If the bytes are a 1 followed by 15, then we are
		//parsing the basic sensor data packet
		kRead = serialGetchar(kobuki);
		if(kRead == 1){
			if(serialGetchar(kobuki) == 15) break;
		}
	}

	//Read past the timestamp
	serialGetchar(kobuki);
	serialGetchar(kobuki);

	/*Read the bytes containing the bumper, wheel drop,
		and cliff sensors. You can convert them into a usable data type.*/
	bumper = (int)serialGetchar(kobuki);
	drop = (int)serialGetchar(kobuki);
	cliff = (int)serialGetchar(kobuki);
	/*Print the data to the screen.*/
	/*Read through 6 bytes between the cliff sensors and
	the button sensors.*/
	for(int i = 0; i < 6; i++) serialGetchar(kobuki);

	/*Read the byte containing the button data.*/
	button = serialGetchar(kobuki);

	cout << "bumper: " << bumper << " drop: " << drop << " cliff: " << cliff << " button: " << button << endl;
}

void backupAndRotate(bool clockwise)
{
	translateSequence(false);
	rotateSequence(clockwise);
}

void rotateSequence(bool clockwise)
{
	int rotMod = clockwise ? 1 : -1;
	float radSpeed = (3.14159f / 2) / 1;	// pi/2rad / 1second
	int rotSpeed = -(int)(radSpeed * (bias / 2));	// w * b / 2
	rotSpeed *= rotMod * lostPackageMod;
	/*Rotate the Kobuki 90 degrees*/
	for (int i = 0; i < 1000 / baud; i++)  
		movement(rotSpeed, 1);
	
	movement(0,0);
}

void translateSequence(bool fwd)
{
	int speedMod = fwd ? 1 : -1;
	int translationSpeed = (int)(speed * speedMod * lostPackageMod);
	for (int i = 0; i < backupTime / baud; i++)
		movement(translationSpeed, 0);
	
	movement(0,0);
}