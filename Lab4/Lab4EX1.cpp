//Use g++ -std=c++11 -o Lab4EX1 Lab4EX1.cpp -lwiringPi

#include <string>
#include <iostream>
#include <wiringSerial.h>
#include <wiringPi.h>
#include <unistd.h>
#include <cstdlib>
#include <iomanip>
using namespace std;

int kobuki;

unsigned int sendRate = 20000;

int main(){
	wiringPiSetup();
	kobuki = serialOpen("/dev/kobuki", 115200);
	unsigned int bumper;
	unsigned int drop;
	unsigned int cliff;
	unsigned int button;
	unsigned int read;

	while(serialDataAvail(kobuki) != -1){

		while(true){
		//If the bytes are a 1 followed by 15, then we are
		//parsing the basic sensor data packet
			read = serialGetchar(kobuki);
			if(read == 1){
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
		cout << "bumper: " << bumper << " drop: " << drop << " cliff: " << cliff << " button: " << button << endl;
		/*Read through 6 bytes between the cliff sensors and
		the button sensors.*/
		for(int i = 0; i < 6; i++) serialGetchar(kobuki);

		/*Read the byte containing the button data.*/
		button = serialGetchar(kobuki);
		/*Close the script and the connection to the Kobuki when
		Button 1 on the Kobuki is pressed. Use serialClose(kobuki);*/
		if (button == 0x02) 
		{
			serialClose(kobuki);
			return 0;	
		}

		//Pause the script so the data read receive rate is the same as the Kobuki send rate.
		usleep(sendRate);
	}

	return 0;
}
