#include "VehicleControl.h"

#include <iostream>
#include <iomanip>

int VehicleControl::connect(String^ hostName, int portNumber) // Establish TCP connection
{
	// Creat TcpClient object and connect to it
	Client = gcnew TcpClient(hostName, portNumber);

	// Configure connection
	Client->NoDelay = true;
	Client->ReceiveTimeout = 500;//ms
	Client->SendTimeout = 500;//ms
	Client->ReceiveBufferSize = 1024;
	Client->SendBufferSize = 1024;

	SendData = gcnew array<unsigned char>(100); // sending the authentication and the control strings(one at a time)
	ReadData = gcnew array<unsigned char>(4);   // receiving the 'OK' string

	// Get the network stream object associated with client so we 
	// can use it to read and write
	Stream = Client->GetStream();

	//-------------Authentication-------------//
	// 
	//String for authentication
	String^ zID = gcnew String("5331003\n");
	// Convert zID string to an array of unsigned char to be used for authentication
	SendData = System::Text::Encoding::ASCII->GetBytes(zID);
	// Write zID to the stream
	Stream->Write(SendData, 0, SendData->Length);
	// Wait for the server to prepare the data, 1 ms would be sufficient, but used 10 ms
	System::Threading::Thread::Sleep(10);
	// Read the incoming data
	Stream->Read(ReadData, 0, ReadData->Length);

	ResponseData = System::Text::Encoding::ASCII->GetString(ReadData); 

	// Print the received string on the screen
	Console::WriteLine(ResponseData); // shud get 'OK\n'

	//------------------------------------------//

	SendData = gcnew array<unsigned char>(30); // reset buffer

	return 1;
}

int VehicleControl::setupSharedMemory() // Create and access shared memory objects
{
	// Use the given SM pointer to point to the SM objects needed
	ProcessManagementData = new SMObject(_TEXT("ProcessManagement"), sizeof(ProcessManagement));
	VehicleControlData = new SMObject(_TEXT("VehicleControl"), sizeof(SM_VehicleControl));

	// Give access to the SM object and check if there are errors
	ProcessManagementData->SMAccess();
	if (ProcessManagementData->SMAccessError) {
		std::cout << "Shared memory access of PMObj failed" << std::endl;
		std::cout << "Press any key to exit/continue..." << std::endl;
		getch();
		return -2;
	}

	VehicleControlData->SMAccess();
	if (VehicleControlData->SMAccessError) {
		std::cout << "Shared memory access of VehicleControlObj failed" << std::endl;
		std::cout << "Press any key to exit/continue..." << std::endl;
		getch();
		return -2;
	}

	// Allocate PM. pointer to pData	
	PMptr = (ProcessManagement*)ProcessManagementData->pData;
	VCptr = (SM_VehicleControl*)VehicleControlData->pData;

	return 1;
}

void VehicleControl::sendCommandToUGV(unsigned int flag)
{
	String^ controlString = gcnew String("# " + VCptr->Steering + " " + VCptr->Speed + " " + flag + " #"); 

	//Prints the sent controls
	std::fixed;
	std::cout << std::setprecision(2);
	std::cout << "Steering: " << VCptr->Steering << "     \tSpeed: " << VCptr->Speed << "    \tFlag : " << flag << std::endl;

	//Sends the controlString to control the UGV
	SendData = System::Text::Encoding::ASCII->GetBytes(controlString);
	Stream->WriteByte(0x02);
	Stream->Write(SendData, 0, SendData->Length);
	Stream->WriteByte(0x03);
}

int VehicleControl::setShutdownFlag(bool shutdown) // Update shutdown signal for module
{
	if (shutdown == 1) {
		PMptr->Shutdown.Flags.VehicleControl = 1;
	}
	else {
		PMptr->Shutdown.Flags.VehicleControl = 0;
	}
	return 1;
}
int VehicleControl::setShutdownStatus(bool shutdown) // Update shutdown status for when it detects that PM died
{
	if (shutdown == 1) {
		PMptr->Shutdown.Status = 0xFF;
	}
	else {
		PMptr->Shutdown.Status = 0x00;
	}
	return 1;
}
bool VehicleControl::getShutdownFlag() // Get Shutdown signal for module, from Process Management SM
{
	return PMptr->Shutdown.Flags.VehicleControl;
}
int VehicleControl::setHeartbeat(bool heartbeat) // Update heartbeat signal for module
{
	PMptr->Heartbeat.Flags.VehicleControl = heartbeat;
	return 1;
}
bool VehicleControl::getHeartbeat() // Get Heartbeat signal for module, from Process Management SM
{
	return PMptr->Heartbeat.Flags.VehicleControl;
}
VehicleControl::~VehicleControl()
{
	Stream->Close();
	Client->Close();
	delete ProcessManagementData;
	delete VehicleControlData;
}