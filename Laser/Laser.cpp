#define _USE_MATH_DEFINES  // M_PI(pi)

#include "Laser.h"
#include <math.h> // sin, cos, M_PI(pi)

int Laser::connect(String^ hostName, int portNumber) // Establish TCP connection
{
	// Creat TcpClient object and connect to it
	std::cout << "1" << std::endl;
	Client = gcnew TcpClient(hostName, portNumber);
	std::cout << "2" << std::endl;
	// Configure connection
	Client->NoDelay = true;
	Client->ReceiveTimeout = 500;//ms
	Client->SendTimeout = 500;//ms
	Client->ReceiveBufferSize = 1024;
	Client->SendBufferSize = 1024;

	// unsigned char arrays are created on managed heap(16bytes and 2500bytes)
	SendData = gcnew array<unsigned char>(16); // only sending the 'ask scan' string which is of 15chars
	ReadData = gcnew array<unsigned char>(2500); //shud be enough to read one scan

	// Get the network stream object associated with client so we 
	// can use it to read and write
	std::cout << "3" << std::endl;
	Stream = Client->GetStream();
	std::cout << "4" << std::endl;
	//-------------Authentication-------------//
	// 
	//String for authentication
	String^ zID = gcnew String("5331003\n");
	// Convert zID string to an array of unsigned char to be used for authentication
	std::cout << "5" << std::endl;
	SendData = System::Text::Encoding::ASCII->GetBytes(zID);
	std::cout << "6" << std::endl;
	// Write zID to the stream
	Stream->Write(SendData, 0, SendData->Length);
	std::cout << "7" << std::endl;
	// Wait for the server to prepare the data, 1 ms would be sufficient, but used 10 ms
	System::Threading::Thread::Sleep(10);
	// Read the incoming data
	Stream->Read(ReadData, 0, ReadData->Length);
	std::cout << "8" << std::endl;
	ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);
	std::cout << "9" << std::endl;
	// Print the received string on the screen
	Console::WriteLine(ResponseData); // should print 'OK'
	if (ResponseData != "OK") {
		Console::WriteLine("User not authorised");
		PMptr->Shutdown.Flags.Laser = 1;
	}

	//------------------------------------------//

	return 1;
}

int Laser::setupSharedMemory() // Create and access shared memory objects
{
	// Use the given SM pointer to point to the SM objects needed
	ProcessManagementData = new SMObject(_TEXT("ProcessManagement"), sizeof(ProcessManagement));
	SensorData = new SMObject(_TEXT("Laser"), sizeof(SM_Laser));
	
	// Give access to the SM object and check if there are errors
	ProcessManagementData->SMAccess();
	if (ProcessManagementData->SMAccessError) {
		std::cout << "Shared memory access of PMObj failed" << std::endl;
		std::cout << "Press any key to exit/continue..." << std::endl;
		getch();
		return -2;
	}

	SensorData->SMAccess();
	if (ProcessManagementData->SMAccessError) {
		std::cout << "Shared memory access of LaserObj failed" << std::endl;
		std::cout << "Press any key to exit/continue..." << std::endl;
		getch();
		return -2;
	}

	// Allocate PM. pointer to pData	
	PMptr = (ProcessManagement*)ProcessManagementData->pData;
	Laserptr = (SM_Laser*)SensorData->pData;

	return 1;
}
int Laser::askForScan() // Ask server for scan
{
	// String command to ask for scan from laser
	AskScan = gcnew String("sRN LMDscandata");
	// Convert string command(asking for scan) to an array of unsigned char(bytes) to be ready to send to laser sensor
	SendData = System::Text::Encoding::ASCII->GetBytes(AskScan);

	// Write command asking for data
	Stream->WriteByte(0x02); //start
	Stream->Write(SendData, 0, SendData->Length);
	Stream->WriteByte(0x03);//end
	// Wait for the server to prepare the data, 1 ms would be sufficient, but used 10 ms
	System::Threading::Thread::Sleep(10);
	// Read the incoming data
	Stream->Read(ReadData, 0, ReadData->Length);
	// Convert incoming data from an array of unsigned char bytes to an ASCII string
	ResponseData = System::Text::Encoding::ASCII->GetString(ReadData);
	LaserDataArray = ResponseData->Split(' '); //Split the string by ' ' and store in array

	// Extract key datas and convert to int32 to be used in calculations of X and Y
	StartAngle = System::Convert::ToInt32(LaserDataArray[23], 16);
	Resolution = System::Convert::ToInt32(LaserDataArray[24], 16) / 10000.0; //shud be 0.5 degree
	AmountOfRanges = System::Convert::ToInt32(LaserDataArray[25], 16); //shud be 361
	Header = LaserDataArray[1]; // shud be sRA

	return 1;
}
int Laser::getData() // Get data from sensor (GPS / Laser) 
{
	
	// Calculation of X and Y
	Range = gcnew array<double>(AmountOfRanges);
	RangeX = gcnew array<double>(AmountOfRanges); //See week5's slides for diagram of the co-ordinates wrt. the actual laser sensor
	RangeY = gcnew array<double>(AmountOfRanges);

	for (int i = 0; i < AmountOfRanges; i++)
	{
		Range[i] = System::Convert::ToInt32(LaserDataArray[26 + i], 16);
		RangeX[i] = Range[i] * sin(i * Resolution * M_PI / 180.0); 
		RangeY[i] = -Range[i] * cos(i * Resolution * M_PI / 180.0);

		// Save the calculated Datas in shared memory structures
		Laserptr->x[i] = RangeX[i];
		Laserptr->y[i] = RangeY[i];

		//Print the received string on the screen
		Console::WriteLine("#", i, "		X: ", RangeX[i], "	Y: ", RangeY[i]);

	}
	
	return 1;
}
bool Laser::checkData() // Check Data is correct (eg. headers and amount of data)
{
	checkFlag = 1;
	if (AmountOfRanges != 361 || Header != "sRA") {
		checkFlag = 0;
	}

	return checkFlag;
}
int Laser::sendDataToSharedMemory() // Save Data in shared memory structures
{
	// IMPLEMENTED UNDER getData()

	return 1;
}
int Laser::setShutdownFlag(bool shutdown) // Update shutdown signal for module
{
	if (shutdown == 1) {
		PMptr->Shutdown.Flags.Laser = 1;
	}
	else {
		PMptr->Shutdown.Flags.Laser = 0;
	}
	return 1;
}
int Laser::setShutdownStatus(bool shutdown) // Update shutdown status for when it detects that PM died
{
	if (shutdown == 1) {
		PMptr->Shutdown.Status = 0xFF;
	}
	else {
		PMptr->Shutdown.Status = 0x00;
	}
	return 1;
}
bool Laser::getShutdownFlag() // Get Shutdown signal for module, from Process Management SM
{
	return PMptr->Shutdown.Flags.Laser;
}
int Laser::setHeartbeat(bool heartbeat) // Update heartbeat signal for module
{
	PMptr->Heartbeat.Flags.Laser = heartbeat;
	return 1;
}
bool Laser::getHeartbeat() // Get Heartbeat signal for module, from Process Management SM
{
	return PMptr->Heartbeat.Flags.Laser;
}
Laser::~Laser()
{
	Stream->Close();
	Client->Close();
	delete ProcessManagementData;
}