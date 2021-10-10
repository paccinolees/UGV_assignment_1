#using <System.dll>
#include <Windows.h>
#include <tchar.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <iostream>
#include <conio.h>

#include "smstructs.h"
#include "SMObject.h"

using namespace System;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;
using namespace System::Threading;

#define NUM_UNITS 5 //no. of modules, EG. if only done GPS.exe change this to 1 etc..

bool IsProcessRunning(const char* processName); 
void StartProcesses();

//defining start up sequence
TCHAR Units[10][20] = //
{
	TEXT("Laser.exe"),
	TEXT("Display.exe"),
	TEXT("VehicleControl.exe"),
	TEXT("GPS.exe"),
	TEXT("Camera.exe")
};

//counter for heartbeat detection //SHOULD ADD CAMERA SOON
int LaserCounter = 0;
int VehicleControlCounter = 0;
int GPSCounter = 0;
int CameraCounter = 0;
int DisplayCounter = 0;
const int max_waitCount = 8; // 2 sec 

int main() {
	// Instantiate SM Objects
	SMObject PMObj(_TEXT("ProcessManagement"), sizeof(ProcessManagement));
	SMObject LaserObj(_TEXT("Laser"), sizeof(SM_Laser));
	SMObject VehicleControlObj(_TEXT("VehicleControl"), sizeof(SM_VehicleControl));
	SMObject GPSObj(_TEXT("GPS"), sizeof(SM_GPS));

	// Create all SM objects and give access to required one(s) and check if there are errors
	PMObj.SMCreate();
	if (PMObj.SMCreateError) {
		std::cout << "Shared memory creation of PMObj failed" << std::endl;
		std::cout << "Press any key to exit/continue..." << std::endl;
		getch();
		return -1;
	}

	PMObj.SMAccess();
	if (PMObj.SMAccessError) {
		std::cout << "Shared memory access of PMObj failed" << std::endl;
		std::cout << "Press any key to exit/continue..." << std::endl;
		getch();
		return -2; 
	}

	LaserObj.SMCreate();
	if (LaserObj.SMCreateError) {
		std::cout << "Shared memory creation of LaserObj failed" << std::endl;
		std::cout << "Press any key to exit/continue..." << std::endl;
		getch();
		return -1;
	}

	VehicleControlObj.SMCreate();
	if (VehicleControlObj.SMCreateError) {
		std::cout << "Shared memory creation of VehicleControlObj failed" << std::endl;
		std::cout << "Press any key to exit/continue..." << std::endl;
		getch();
		return -1;
	}

	GPSObj.SMCreate();
	if (GPSObj.SMCreateError) {
		std::cout << "Shared memory creation of GPSObj failed" << std::endl;
		std::cout << "Press any key to exit/continue..." << std::endl;
		getch();
		return -1;
	}

	// Allocate PM. pointer to pData
	ProcessManagement* PMptr;
	PMptr = (ProcessManagement*)PMObj.pData;

	// Initialize heartbeat and shutdown statuses
	PMptr->Shutdown.Status = 0x00;
	PMptr->Heartbeat.Status = 0x00;

	//start all 5 modules using CLR
	StartProcesses();

	//main loop
	while (!PMptr->Shutdown.Flags.ProcessManagement) {
		Thread::Sleep(250);

		// Changes other module's heartbeat to 0, if heartbeat from other modules not detected: add counter 
		if (PMptr->Heartbeat.Flags.Laser == 1) {
			PMptr->Heartbeat.Flags.Laser = 0;
			LaserCounter = 0;
		}
		else {
			LaserCounter += 1;
		}
		if (PMptr->Heartbeat.Flags.VehicleControl == 1) {
			PMptr->Heartbeat.Flags.VehicleControl = 0;
			VehicleControlCounter = 0;
		}
		else {
			VehicleControlCounter += 1;
		}
		if (PMptr->Heartbeat.Flags.GPS == 1) {
			PMptr->Heartbeat.Flags.GPS = 0;
			GPSCounter = 0;
		}
		else {
			GPSCounter += 1;
		}
		if (PMptr->Heartbeat.Flags.Camera == 1) {
			PMptr->Heartbeat.Flags.Camera = 0;
			CameraCounter = 0;
		}
		else {
			CameraCounter += 1;
		}
		if (PMptr->Heartbeat.Flags.OpenGL == 1) {
			PMptr->Heartbeat.Flags.OpenGL = 0;
			DisplayCounter = 0;
		}
		else {
			DisplayCounter += 1;
		}
		
		// Attempt to restart non-crit processes
		if (GPSCounter > max_waitCount) {
			std::cout << "GPS(non-crit.) failed, attempting restart..." << std::endl;
			PMptr->Shutdown.Flags.GPS = 1;
			Thread::Sleep(30); //Allow time for GPS to receive the shutdown signal and shut itself down
			StartProcesses(); //Restart
			GPSCounter = 0;
		}
		if (DisplayCounter > max_waitCount) {
			std::cout << "Display(non-crit.) failed, attempting restart..." << std::endl;
			PMptr->Shutdown.Flags.OpenGL = 1;
			Thread::Sleep(30); //Allow time for GPS to receive the shutdown signal and shut itself down
			StartProcesses(); //Restart
			DisplayCounter = 0;
		}

		//Shutdown routine (kbhit or critical processes failed)
		if (_kbhit() || LaserCounter > max_waitCount /* || CameraCounter > max_waitCount || VehicleControlCounter > max_waitCount*/) {
			PMptr->Shutdown.Status = 0xFF;
			std::cout << "SHUTDOWN routine activated by kbhit/failure of critical processes" << std::endl;
		}
		//added for debugging, DEL LATER
		if (LaserCounter > max_waitCount) {
			PMptr->Shutdown.Status = 0xFF;
			std::cout << "Laser failed...shutting down" << std::endl;
		}
		if (CameraCounter > max_waitCount) {
			PMptr->Shutdown.Status = 0xFF;
			std::cout << "Camera failed...shutting down" << std::endl;
		}
		if (VehicleControlCounter > max_waitCount) {
			PMptr->Shutdown.Status = 0xFF;
			std::cout << "VehicleControl failed...shutting down" << std::endl;
		}
	
	} 
	//ASK LATER IF I SHUD INPUT A DELAY LIKE THIS
	std::cout << "Process Manager terminating in 3 seconds..." << std::endl;
	Thread::Sleep(3000);

	return 0;
}



//Is process running function
bool IsProcessRunning(const char* processName)
{
	bool exists = false;
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry))
		while (Process32Next(snapshot, &entry))
			if (!_stricmp((const char *)entry.szExeFile, processName))
				exists = true;

	CloseHandle(snapshot);
	return exists;
}


void StartProcesses()
{
	STARTUPINFO s[10];
	PROCESS_INFORMATION p[10];

	for (int i = 0; i < NUM_UNITS; i++)
	{
		if (!IsProcessRunning((const char *)Units[i]))
		{
			ZeroMemory(&s[i], sizeof(s[i]));
			s[i].cb = sizeof(s[i]);
			ZeroMemory(&p[i], sizeof(p[i]));

			if (!CreateProcess(NULL, Units[i], NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &s[i], &p[i]))
			{
				printf("%s failed (%d).\n", Units[i], GetLastError());
				_getch();
			}
			std::cout << "Started: " << Units[i] << std::endl;
			Sleep(100);
		}
	}
}


