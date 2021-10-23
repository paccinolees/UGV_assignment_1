#using <System.dll>
#include <Windows.h>
#include <tchar.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <iostream>

#include "smstructs.h"
#include "SMObject.h"
#include "VehicleControl.h"

using namespace System::Threading;

//counter for heartbeat detection 
int PMCounter = 0;
const int max_waitCount = 100;  //2.5sec

int main()
{
	// Create a Laser object
	VehicleControl myVCObj;

	// Setup SM (Give access to SM objects and check if there are errors)
	myVCObj.setupSharedMemory();

	// Initialize shutdown status
	myVCObj.setShutdownFlag(0);
	// Connect to laser sensor(Authentication process)
	myVCObj.connect("192.168.1.200", 25000);

	//-------------MAIN LOOP-------------//
	unsigned int flagForSendCommand = 0;
	while (!myVCObj.getShutdownFlag()) {
		// Changes its heartbeat to 1, if PM doesn't change it back to 0, add PMCounter
		if (!myVCObj.getHeartbeat()) {
			myVCObj.setHeartbeat(1);
			PMCounter = 0;
		}
		else {
			PMCounter += 1;

			if (PMCounter > max_waitCount) {
				std::cout << "PM failed, sending shutdown signal..." << std::endl;
				myVCObj.setShutdownStatus(1);
			}
		}

		myVCObj.sendCommandToUGV(flagForSendCommand);
		flagForSendCommand = !flagForSendCommand;

		Thread::Sleep(25);
	}

	std::cout << "VehicleControl process terminating..." << std::endl;

	return 0;
}