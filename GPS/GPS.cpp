#include "GPS.h"
#pragma pack(1)

struct GPSstruct // 112 bytes
{
	unsigned int Header;
	unsigned char Discards1[40];
	double Northing;
	double Easting;
	double Height;
	unsigned char Discards2[40];
	unsigned int Checksum;
};
GPSstruct NovatelGPS; // Create the struct's object

int GPS::connect(String^ hostName, int portNumber)
{
	// Creat TcpClient object and connect to it
	Client = gcnew TcpClient(hostName, portNumber);

	// Configure connection (copied from Laser)
	Client->NoDelay = true;
	Client->ReceiveTimeout = 500;//ms
	Client->SendTimeout = 500;//ms
	Client->ReceiveBufferSize = 1024;
	Client->SendBufferSize = 1024;

	// unsigned char array is created on managed heap
	ReadData = gcnew array<unsigned char>(225); // >twice the size of data packet(112) (?)

	// Get the network stream object associated with client so we 
	// can use it to read and write
	Stream = Client->GetStream();

	return 1;
}
int GPS::setupSharedMemory()
{
	// Use the given SM pointer to point to the SM objects needed
	ProcessManagementData = new SMObject(_TEXT("ProcessManagement"), sizeof(ProcessManagement));
	SensorData = new SMObject(_TEXT("GPS"), sizeof(SM_GPS));

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
		std::cout << "Shared memory access of GPSObj failed" << std::endl;
		std::cout << "Press any key to exit/continue..." << std::endl;
		getch();
		return -2;
	}

	// Allocate PM. pointer to pData	
	PMptr = (ProcessManagement*)ProcessManagementData->pData;
	GPSptr = (SM_GPS*)SensorData->pData;

	return 1;
}
int GPS::askForScan()
{
	// Wait for the server to prepare the data, 1 ms would be sufficient, but used 10 ms (copied from Laser)
	System::Threading::Thread::Sleep(10);
	// Read the incoming data if it is available
	if (Stream->DataAvailable) {
		Stream->Read(ReadData, 0, ReadData->Length);
	}

	return 1;
}
bool GPS::checkData() // trapping header AND store the data in the created struct
{
	bool validHeader = false;
	unsigned int Header = 0;
	unsigned char Data; // to store each byte that was read
	int i = 0;
	int Start; // index indicating the start of the actual data(after the header)

	do
	{
		Data = ReadData[i++];
		Header = ((Header << 8) | Data);
		if (Header == 0xaa44121c) {
			validHeader = true;
		}
		if (i >= 225) { // exceeding size of array
			break;
		}
	} while (Header != 0xaa44121c);

	if (validHeader == false) {
		return 0;
	}

	Start = i - 4;

	// Storing datas(after the header) into the created struct
	unsigned char* byteptr = nullptr;
	byteptr = (unsigned char*)&NovatelGPS;
	for (int i = Start; i < Start + sizeof(NovatelGPS); i++)
	{
		*(byteptr++) = ReadData[i];
	}

	return 1;
}
bool GPS::checkCRC() // Comparing CRC of the data from the struct between the actual one and the calculated one
{
	unsigned char* byteptr2 = (unsigned char*)&NovatelGPS;
	calculatedCRC = CalculateBlockCRC32(108 /*bytes of all data except the checksum itself*/, byteptr2);

	checkCRC_flag = 0;

	if (calculatedCRC == NovatelGPS.Checksum) {
		checkCRC_flag = 1;
	}

	return checkCRC_flag;
}
int GPS::getData() //prints the important datas (after the checks passed)
{
	easting = NovatelGPS.Easting;
	northing = NovatelGPS.Northing;
	height = NovatelGPS.Height;

	return 1;
}
void GPS::printData()
{
	Console::Write("Northing: {0,10:F3}" + "	Easting: {1, 10:F3}" + "	Height: {2, 10:F3}\n", northing, easting, height);
}
int GPS::sendDataToSharedMemory()
{
	GPSptr->northing = northing;
	GPSptr->easting = easting;
	GPSptr->height = height;

	return 1;
}
int GPS::setShutdownFlag(bool shutdown) // Update shutdown signal for module
{
	if (shutdown == 1) {
		PMptr->Shutdown.Flags.GPS = 1;
	}
	else {
		PMptr->Shutdown.Flags.GPS = 0;
	}
	return 1;
}
int GPS::setShutdownStatus(bool shutdown) // Update shutdown status for when it detects that PM died
{
	if (shutdown == 1) {
		PMptr->Shutdown.Status = 0xFF;
	}
	else {
		PMptr->Shutdown.Status = 0x00;
	}
	return 1;
}
bool GPS::getShutdownFlag() // Get Shutdown signal for module, from Process Management SM
{
	return PMptr->Shutdown.Flags.GPS;
}
int GPS::setHeartbeat(bool heartbeat) // Update heartbeat signal for module
{
	PMptr->Heartbeat.Flags.GPS = heartbeat;
	return 1;
}
bool GPS::getHeartbeat() // Get Heartbeat signal for module, from Process Management SM
{
	return PMptr->Heartbeat.Flags.GPS;
}
GPS::~GPS()
{
	Stream->Close();
	Client->Close();
	delete ProcessManagementData;
	delete SensorData;
}


unsigned long CRC32Value(int i)
{
	int j;
	unsigned long ulCRC;
	ulCRC = i;
	for (j = 8; j > 0; j--)
	{
		if (ulCRC & 1)
			ulCRC = (ulCRC >> 1) ^ CRC32_POLYNOMIAL;
		else
			ulCRC >>= 1;
	}
	return ulCRC;
}

unsigned long CalculateBlockCRC32(unsigned long ulCount, /* Number of bytes in the data block */
	unsigned char* ucBuffer) /* Data block */
{
	unsigned long ulTemp1;
	unsigned long ulTemp2;
	unsigned long ulCRC = 0;
	while (ulCount-- != 0)
	{
		ulTemp1 = (ulCRC >> 8) & 0x00FFFFFFL;
		ulTemp2 = CRC32Value(((int)ulCRC ^ *ucBuffer++) & 0xff);
		ulCRC = ulTemp1 ^ ulTemp2;
	}
	return(ulCRC);
}
