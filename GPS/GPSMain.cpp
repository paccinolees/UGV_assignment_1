#using <System.dll>
#include <Windows.h>
#include <tchar.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <iostream>

#include "smstructs.h"
#include "SMObject.h"
#include "GPS.h"

using namespace System::Threading;

//counter for heartbeat detection 
int PMCounter = 0;
const int max_waitCount = 100;  //2.5sec

int main()
{
	// Instantiate SM Objects
	SMObject PMObj(_TEXT("ProcessManagement"), sizeof(ProcessManagement));
	SMObject GPSObj(_TEXT("GPS"), sizeof(SM_GPS));

	// Give access to SM objects and check if there are errors
	PMObj.SMAccess();
	if (PMObj.SMAccessError) {
		std::cout << "Shared memory access of PMObj failed" << std::endl;
		std::cout << "Press any key to exit/continue..." << std::endl;
		getch();
		return -2;
	}
	GPSObj.SMAccess();
	if (GPSObj.SMAccessError) {
		std::cout << "Shared memory access of GPSObj failed" << std::endl;
		std::cout << "Press any key to exit/continue..." << std::endl;
		getch();
		return -2;
	}

	// Allocate PM. pointer to pData
	ProcessManagement* PMptr;
	SM_GPS* GPSptr;
	PMptr = (ProcessManagement*)PMObj.pData;
	GPSptr = (SM_GPS*)GPSObj.pData;

	// Initialize shutdown status
	PMptr->Shutdown.Flags.GPS = 0;

	while (!PMptr->Shutdown.Flags.GPS) {
		// Changes its heartbeat to 1, if PM doesn't change it back to 0, add PMCounter
		if (PMptr->Heartbeat.Flags.GPS == 0) {
			PMptr->Heartbeat.Flags.GPS = 1;
			PMCounter = 0;
		}
		else {
			PMCounter += 1;

			if (PMCounter > max_waitCount) {
				std::cout << "PM failed, sending shutdown signal..." << std::endl;
				PMptr->Shutdown.Status = 0xFF;
			}	
		}
	

		Thread::Sleep(25);
	}

	std::cout << "GPS process terminating..." << std::endl;
	return 0;
}