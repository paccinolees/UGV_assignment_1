#ifndef __MYVEHICLE_HPP__
#define __MYVEHICLE_HPP__


#include "Vehicle.hpp"

class MyVehicle : public Vehicle
{
public:
	//MyVehicle(int AmountOfRanges, double x[], double y[]);
	MyVehicle(int* AmountOfRanges, double x[], double y[]);
	virtual void draw();
	void drawLaserScans();
	
protected:
	int* NumOfRanges;
	double* xRange;
	double* yRange;
};

#endif