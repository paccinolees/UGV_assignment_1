#pragma once

#include <UGV_module.h>
#include <smstructs.h>


ref class VehicleControl : public UGV_module //VehicleControl class created by inheriting from 'UGV_module' class
{

public:
	int connect(String^ hostName, int portNumber) override;
	int setupSharedMemory() override;
	void sendCommandToUGV(unsigned int flag);
	int setShutdownFlag(bool shutdown) override;
	int setShutdownStatus(bool shutdown) override;
	bool getShutdownFlag() override;
	int setHeartbeat(bool heartbeat) override;
	bool getHeartbeat() override;
	~VehicleControl();

protected: //All added by me
	SM_VehicleControl* VCptr;									// SM_VehicleControl pointer
	SMObject* VehicleControlData;								
};
