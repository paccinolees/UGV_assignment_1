#pragma once

#include <UGV_module.h>
#include <smstructs.h>

#define CRC32_POLYNOMIAL 0xEDB88320L

unsigned long CRC32Value(int i);
unsigned long CalculateBlockCRC32(unsigned long ulCount, unsigned char* ucBuffer);

ref class GPS : public UGV_module //GPS class created by inheriting from 'UGV_module' class
{

public:
	int connect(String^ hostName, int portNumber) override;
	int setupSharedMemory() override;
	int askForScan() override;
	int getData() override;
	bool checkData() override;
	int sendDataToSharedMemory() override;
	bool getShutdownFlag() override;
	int setHeartbeat(bool heartbeat) override;
	int setShutdownFlag(bool shutdown) override;
	int setShutdownStatus(bool shutdown) override;
	bool getHeartbeat() override;
	bool checkCRC(); //added just in GPS.h
	void printData(); //added just in GPS.h
	~GPS();

protected: //All added by me
	SM_GPS* GPSptr;									// SM_GPS pointer		
	bool checkCRC_flag;								// flag for checkCRC()
	unsigned int calculatedCRC;						// Calculated CRC value using "CalculateBlockCRC32" function
	double northing;
	double easting;
	double height;
};
