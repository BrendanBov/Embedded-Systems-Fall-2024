//Use g++ -std=c++11 -o Lab3BonusB Lab3BonusB.cpp -lwiringPi

#include <stdio.h>
#include <iostream>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include "joystick.hh"
#include <cstdlib>
#define  PORT 8080
using namespace std;

int createSocket();

int sock = 0;
const int baud = 20;

const int dPadXAxis = 6;
const int dPadYAxis = 7; 
const int startBtn = 7;
const int selectBtn = 8;

int main(int argc, char const *argv[]){
	
	//Open the file stream for the joystick
	Joystick joystick("/dev/input/js0");
	JoystickEvent event;
	if(!joystick.isFound()){
		cout << "Error opening joystick" << endl;
		exit(1);
	}


	//Create the connection to the server
	createSocket();

	while(true){

		/*Sample the events from the joystick*/
		int eventRaised = joystick.sample(&event);
		int isButton = event.isButton();
		int isJoystick = event.isAxis();
		int inputNumber = event.number;
		int inputValue = event.value;
		/*Convert the event to a useable data type so it can be sent*/
		int toSend[5] = {eventRaised, isButton, isJoystick, inputNumber, inputValue};
		//strcpy(toSend,"hihi");
		/*Print the data stream to the terminal*/
		cout << "input event: " << eventRaised << ", is button: " << isButton << ", is joystick: " << isJoystick << ", input number: " << inputNumber << ", input value: " << inputValue << endl;
		/*Send the data to the server*/
		send(sock, toSend, sizeof(toSend), 0);
		if(eventRaised && isButton && inputNumber == selectBtn && inputValue == 1) {
			/*Closes out of all connections cleanly*/

			//When you need to close out of the connection, please
			//close TTP/IP data streams.
			//Not doing so will result in the need to restart
			//the raspberry pi and Kobuki
			sleep(1);	// wait for message to send

			close(sock);
			exit(0);

			/*Set a delay*/
			
		}
		usleep(baud * 1000);
	}
	return 0;
}

//Creates the connection between the client and
//the server with the controller being the client
int createSocket(){
	struct sockaddr_in address;
	struct sockaddr_in serv_addr;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		printf("\nSocket creation error \n");
		return -1;
	}

	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port   = htons(PORT);

	/*Use the IP address of the server you are connecting to*/
	if(inet_pton(AF_INET, "127.0.0.1" , &serv_addr.sin_addr) <= 0){
		printf("\nInvalid address/ Address not supported \n");
		return -1;
	}

	if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
		printf("\nConnection Failed \n");
		return -1;
	}
	return 0;
}

