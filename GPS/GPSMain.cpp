#using <System.dll>
#include <Windows.h>
#include <tchar.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <iostream>

#include "smstructs.h"
#include "SMObject.h"
#include "GPS.h"

using namespace System;
using namespace System::Threading;

//counter for heartbeat detection 
int PMCounter = 0;
const int max_waitCount = 100;  //2.5sec

int main()
{
	// Create a GPS object
	GPS myGPSObj;

	// Setup SM (Give access to SM objects and check if there are errors)
	myGPSObj.setupSharedMemory();

	// Initialize shutdown status
	myGPSObj.setShutdownFlag(0);
	// Connect to laser sensor(Authentication process)
	myGPSObj.connect("192.168.1.200", 24000);

	//-------------MAIN LOOP-------------//

	while (!myGPSObj.getShutdownFlag()) {
		// Changes its heartbeat to 1, if PM doesn't change it back to 0, add PMCounter
		if (!myGPSObj.getHeartbeat()) {
			myGPSObj.setHeartbeat(1);
			PMCounter = 0;
		}
		else {
			PMCounter += 1;

			if (PMCounter > max_waitCount) {
				std::cout << "PM failed, sending shutdown signal..." << std::endl;
				myGPSObj.setShutdownStatus(1);
			}
		}

		// Scan and check its length
		myGPSObj.askForScan();

		if (!myGPSObj.checkData()) {
			Console::WriteLine("skipping this scan...(Header check failed)");
			continue; // skips the current scan and retry
		}
		if (!myGPSObj.checkCRC()) {
			Console::WriteLine("skipping this scan...(CRC check failed)");
			continue; // skips the current scan and retry
		}

		myGPSObj.getData(); // get the important datas and store it in the member variables
		myGPSObj.printData(); // Prints those important datas
		myGPSObj.sendDataToSharedMemory(); // send datas to SM structure

		Thread::Sleep(25);
	}

	std::cout << "GPS process terminating..." << std::endl;

	return 0;
}
