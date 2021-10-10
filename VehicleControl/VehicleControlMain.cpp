#using <System.dll>
#include <Windows.h>
#include <tchar.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <iostream>

#include "smstructs.h"
#include "SMObject.h"

using namespace System::Threading;

//counter for heartbeat detection 
int PMCounter = 0;
const int max_waitCount = 100;  //2.5sec

int main()
{
	// Instantiate SM Objects
	SMObject PMObj(_TEXT("ProcessManagement"), sizeof(ProcessManagement));
	SMObject VehicleControlObj(_TEXT("VehicleControl"), sizeof(SM_VehicleControl));

	// Give access to SM objects and check if there are errors
	PMObj.SMAccess();
	if (PMObj.SMAccessError) {
		std::cout << "Shared memory access of PMObj failed" << std::endl;
		std::cout << "Press any key to exit/continue..." << std::endl;
		getch();
		return -2;
	}
	VehicleControlObj.SMAccess();
	if (VehicleControlObj.SMAccessError) {
		std::cout << "Shared memory access of VehicleControlObj failed" << std::endl;
		std::cout << "Press any key to exit/continue..." << std::endl;
		getch();
		return -2;
	}

	// Allocate PM. pointer to pData
	ProcessManagement* PMptr;
	SM_VehicleControl* VehicleControlptr;
	PMptr = (ProcessManagement*)PMObj.pData;
	VehicleControlptr = (SM_VehicleControl*)VehicleControlObj.pData;

	// Initialize shutdown status
	PMptr->Shutdown.Flags.VehicleControl = 0;

	while (!PMptr->Shutdown.Flags.VehicleControl) {
		// Changes its heartbeat to 1, if PM doesn't change it back to 0, add PMCounter
		if (PMptr->Heartbeat.Flags.VehicleControl == 0) {
			PMptr->Heartbeat.Flags.VehicleControl = 1;
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

	std::cout << "VehicleControl process terminating..." << std::endl;
	return 0;
}