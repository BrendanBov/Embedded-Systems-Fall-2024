//Use g++ -std=c++11 -o Lab3EX3A Lab3EX3A.cpp -lwiringPi

#include <iostream>
#include <iomanip>
#include <unistd.h>
#include <cstdlib>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <wiringSerial.h>
#include <wiringPi.h>
#define PORT 8080
using namespace std;

void movement(int, int);
void rotateSequence(bool);
void translateSequence(bool);
bool stopMovement();
void createSocket();
void readData();

int kobuki, new_socket;

int bufferLength = 5;
int buffer[5] = {0};	//0: pressed, 1: isBtn, 2: isJoy, 3: inNum, 4: inVal

int eventRaised = 0;
int isButton = 0;
int isJoystick = 0;
int inputNumber = 0;
int inputValue = 0;

//kubuki motor control constants
const int baud = 20;
const int r = 500;
const int bias = 230;
const int speed = 200;
const float lostPackageMod = 1.35f;

const int dPadXAxis = 6;
const int dPadYAxis = 7; 
const int startBtn = 7;
const int selectBtn = 8;

bool runFlag = true;

/*Create char buffer to store transmitted data*/

int main(){
	//Initialize filestream for the Kobuki
	wiringPiSetup();
	kobuki = serialOpen("/dev/kobuki", 115200);

	//Create connection to client
	createSocket();

	while(runFlag){
		//Read data from client
		readData();

		if (eventRaised)
		{
			if (isButton)
			{
				if(stopMovement()) break;
			}
			if (isJoystick)
			{
				if (inputNumber == dPadXAxis)
					rotateSequence(inputValue > 0);	// > 0, clockwise
				else if (inputNumber == dPadYAxis)
					translateSequence(inputValue < 0);
			}
		}
		else
		{
			movement(0,0);
		}

		//usleep(baud * 1000);
	}
	return 0;
}

//Creates the connection between the client and
//the server with the Kobuki being the server
void createSocket(){
	int server_fd;
	struct sockaddr_in address;
	int opt = 1;
	int addrlen = sizeof(address);

	if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))){
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	address.sin_family      = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port        = htons(PORT);

	if(bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0){
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	if(listen(server_fd, 3) < 0){
		perror("listen");
		exit(EXIT_FAILURE);
	}

	if((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0){
		perror("accept");
		exit(EXIT_FAILURE);
	}
}

void readData(){
	/*Read the incoming data stream from the controller*/
	int valread = read(new_socket, buffer, sizeof(buffer));
	eventRaised = buffer[0];
	isButton = buffer[1];
	isJoystick = buffer[2];
	inputNumber = buffer[3];
	inputValue = buffer[4];
	/*Print the data to the terminal*/
	//cout << buffer << endl;
	cout << "input event: " << eventRaised << ", is button: " << isButton << ", is joystick: " << isJoystick << ", input number: " << inputNumber << ", input value: " << inputValue << endl;
	/*Use the received data to control the Kobuki*/
	/*
	if(eventRaised && isButton && inputNumber == selectBtn && inputValue == 1) {
	//Closes out of all connections cleanly

	//When you need to close out of all connections, please
	//close both the Kobuki and TTP/IP data streams.
	//Not doing so will result in the need to restart
	//the raspberry pi and Kobuki
		close(new_socket);
		serialClose(kobuki);
		exit(0);
	}*/

	/*Reset the buffer*/
	//memset(&buffer, '0', sizeof(buffer));

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
		readData();
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
		readData();
		if (isJoystick && inputNumber == dPadYAxis)
			if (inputValue == 0) return;
		if (stopMovement()) return;
		movement(translationSpeed, 0);
	}
	movement(0,0);
}

bool stopMovement()
{

	if (isButton)	//may cause stop on release and start
	{
		if (inputNumber == startBtn && inputValue == 1) return true;
		if (inputNumber == selectBtn && inputValue == 1) 
		{
			close(new_socket);
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
	//delay(baud);	// data rate of 20ms
	usleep(baud * 1000);
}

