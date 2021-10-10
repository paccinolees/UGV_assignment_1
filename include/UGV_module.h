/*****************
UGV_module.h
JAMES STEVENS 2021
*****************/

/*
This file defines the base class for the UGV modules. Laser, GPS, Vehicle Control etc. will be derived from this class
Note that the Camera and OpenGL modules are not derived from this class. For those two, you are only required to modify the existing code
For all other modules, be sure to use derive from this class. You are welcome to modify this file and add any functions / data that
you see fit.
*/

#ifndef MTRN_3500_module
#define MTRN_3500_module

// ERROR HANDLING. Use these as return values in your functions
#define SUCCESS 0
#define ERR_NO_DATA 1
#define ERR_INVALID_DATA 2

#include <iostream>
#include <SMObject.h>
#include <SMFcn.h>
#include <smstructs.h> //added by me
#using <System.dll>

using namespace System;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;

ref class UGV_module
{
	public:
		virtual int connect(String^ hostName, int portNumber) = 0;	// Establish TCP connection
		virtual int setupSharedMemory() = 0;						// Create and access shared memory objects
		virtual int askForScan() = 0;								// Ask server for scan											//added by me
		virtual int getData() = 0;									// Get data from sensor (GPS / Laser)
		virtual bool checkData() = 0;								// Check Data is correct (eg headers)							/changed(to bool) by me
		virtual int sendDataToSharedMemory() = 0;					// Save Data in shared memory structures
		virtual int setShutdownFlag(bool shutdown) = 0;				// Update shutdown signal for module							//added by me
		virtual int setShutdownStatus(bool shutdown) = 0;			// Update shutdown status									    //added by me
		virtual bool getShutdownFlag() = 0;							// Get Shutdown signal for module, from Process Management SM
		virtual int setHeartbeat(bool heartbeat) = 0;				// Update heartbeat signal for module
		virtual bool getHeartbeat() = 0;							// Get Heartbeat signal for module, from Process Management SM	//added by me

		void getError()
		{
			switch (Error)
			{
				case SUCCESS:
					std::cout << "Success." << std::endl;
					break;
				case ERR_NO_DATA:
					std::cout << "ERROR: No Data Available." << std::endl;
					break;
				case ERR_INVALID_DATA:
					std::cout << "ERROR: Invalid Data Received." << std::endl;
					break;
			}
		}


	protected:
		int Error;							// Stores last error code
		TcpClient^ Client;					// Handle for TCP connection
		NetworkStream^ Stream;				// Handle for TCP data stream
		array<unsigned char>^ ReadData;		// Array to store sensor Data
		array<unsigned char>^ SendData;     // Array to send Data (EG. ask for scan)		//added by me
		String^ AskScan;					// String command to ask for scan				//added by me
		String^ ResponseData;				// String to store received data for display	//added by me
		String^ Header;						// Header string from the received data			//added by me

		SMObject* ProcessManagementData;	// Filled in setupSharedMemory(). For accessing PM shared Memory
		SMObject* SensorData;				// Filled in setupSharedMemory(). For storing sensory data to shared Memory
		ProcessManagement* PMptr;			// PM pointer									//added by me
};

#endif 
