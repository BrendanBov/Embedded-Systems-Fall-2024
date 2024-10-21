//use g++ -std=c++11 -o Lab3EX1 Lab3EX1.cpp -lwiringPi

#include <iostream>
#include <unistd.h>
#include <wiringSerial.h>
#include <wiringPi.h>
using namespace std;

void movement(int, int);
int kobuki;

const int r = 500;
const int bias = 230;
const int speed = 200;
const int baud = 20;

int main(){
	wiringPiSetup();
	kobuki = serialOpen("/dev/kobuki", 115200);

	//The Kobuki accepts data packets at a rate of 20 ms.
	//To continually move, data needs to be sent continuously. Therefore, 
	//you need to call void movement(int sp, int r) in a for or while loop 
	//in order to run a specific distance.
	
	//Due to machine error, the calculated value of the time needed
	//will not be exact, but can give you a rough starting value.

	// stop the robot movement between each movement segment.

	float radSpeed = (3.14159f / 2) / 1;	// pi/2rad / 1second
	int rotSpeed = -(int)(radSpeed * (bias / 2));	// w * b / 2
	/*Rotate the Kobuki 90 degrees*/
	for (int i = 0; i < 1350 / baud; i++) movement(rotSpeed, 1);
	
	movement(0,0);

	int translationSpeed = r;
	/*Move along the vertical side*/
	for (int i = 0; i < 1350 / baud; i++) movement(translationSpeed, 0);

	movement(0,0);

	/*Move along quarter circle*/
	int negativeRad = -r;
	int transRotSpeed = (int)(negativeRad * ((negativeRad - bias) / 2.0f) / negativeRad) * 2;
	for (int i = 0; i < 1350 / baud; i++) movement(-transRotSpeed, negativeRad);

	movement(0,0);


	//Close the serial stream to the Kobuki
	//Not doing this will result in an unclean disconnect
	//resulting in the need to restart the Kobuki
	//after every run.
	serialClose(kobuki);
	return 0;
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

