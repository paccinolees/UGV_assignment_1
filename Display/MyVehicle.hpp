#ifndef __MYVEHICLE_HPP__
#define __MYVEHICLE_HPP__


#include "Vehicle.hpp"

class MyVehicle : public Vehicle
{
public:
	MyVehicle(int AmountOfRanges, double x[], double y[]);
	virtual void draw();
	void drawLaserScans();
	//void updateLaserScans(int AmountOfRanges, double x[], double y[]);

protected:
	int NumOfRanges;
	double yRange[361];
	double xRange[361];
};

#endif