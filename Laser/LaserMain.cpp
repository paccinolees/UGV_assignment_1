#using <System.dll>
#include <Windows.h>
#include <tchar.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <iostream>
#include <conio.h> //kbhit() and getch()
#include <math.h> //sin cos

#include "smstructs.h"
#include "SMObject.h"
#include "Laser.h"

using namespace System;
using namespace System::Net::Sockets; 
using namespace System::Net; //For TcpClient
using namespace System::Text; //For converting between bytes and ASCII 
using namespace System::Threading;

//counter for heartbeat detection 
int PMCounter = 0;
const int max_waitCount = 100;  //2.5sec


int main() {
	// Create a Laser object
	Laser myLaserObj;
	
	// Setup SM (Give access to SM objects and check if there are errors)
	myLaserObj.setupSharedMemory();

	// Initialize shutdown status
	myLaserObj.setShutdownFlag(0);
	// Connect to laser sensor(Authentication process)
	std::cout << "BEFORE connect" << std::endl;
	myLaserObj.connect("192.168.1.200", 23000);
	std::cout << "AFTER connect" << std::endl;
	Thread::Sleep(3000);

	//-------------MAIN LOOP-------------//

	while (!myLaserObj.getShutdownFlag()) {
		// Changes its heartbeat to 1, if PM doesn't change it back to 0, add PMCounter
		if (!myLaserObj.getHeartbeat()) {
			myLaserObj.setHeartbeat(1);
			PMCounter = 0;
		}
		else {
			PMCounter += 1;

			if (PMCounter > max_waitCount) {
				std::cout << "PM failed, sending shutdown signal..." << std::endl;
				myLaserObj.setShutdownStatus(1);
			}
		}

		// Scan and output
		myLaserObj.askForScan(); 
		if (!myLaserObj.checkData()) { 
			continue; // skips the current scan and retry
		}
		myLaserObj.getData(); // get calculated data and prints

		Thread::Sleep(25);
	}

	std::cout << "Laser process terminating..." << std::endl;
	//NOTE TO SELF: DONT FORGET TO CHANGE SOLUTION'S PROPERTY'S CONFIG. PROP. and change to 'release' & 'x64' to avoid crashes.
	return 0;
}

