#pragma once

#include <UGV_module.h>
#include <smstructs.h>


ref class Laser : public UGV_module //Laser class created by inheriting from 'UGV_module' class
{

public:
	int connect(String^ hostName, int portNumber) override;
	int setupSharedMemory() override;
	int askForScan() override;
	int extractData() override;
	int getData() override;
	bool checkArrayLength() override;
	bool checkData() override;
	int sendDataToSharedMemory() override;
	int setShutdownFlag(bool shutdown) override;
	int setShutdownStatus(bool shutdown) override;
	bool getShutdownFlag() override;
	int setHeartbeat(bool heartbeat) override;
	bool getHeartbeat() override;
	~Laser();

protected: //All added by me
	SM_Laser* Laserptr;									// SM_Laser pointer						
	array<String^>^ LaserDataArray = nullptr;			// Array of Laser data separated by ' '	
	double StartAngle;									// Start angle in degree
	double Resolution;									// Angular step wideth in degree
	int AmountOfRanges;									// Amount of range-values scanned
	array<double>^ Range;								// Range-values scanned by laser
	array<double>^ RangeX;								// Calculated cartesian X of the range-values in mm
	array<double>^ RangeY;								// Calculated cartesian Y of the range-values in mm
};