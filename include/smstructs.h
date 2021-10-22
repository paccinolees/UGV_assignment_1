#pragma once

#ifndef SMSTRUCTS_H
#define SMSTRUCTS_H

#using <System.dll>
#include <Windows.h>
#include <tchar.h>
#include <TlHelp32.h>
#include <stdio.h>
#include <iostream>
#include <conio.h>

using namespace System;
using namespace System::Net::Sockets;
using namespace System::Net;
using namespace System::Text;


#define STANDARD_LASER_LENGTH 361

struct SM_Laser
{
	double x[STANDARD_LASER_LENGTH];
	double y[STANDARD_LASER_LENGTH];
	int AmountOfRange;
};

struct SM_VehicleControl
{
	double Speed;
	double Steering;
};

struct SM_GPS
{
	double northing;
	double easting;
	double height;
};

struct UnitFlags
{
	unsigned char	ProcessManagement : 1,	//CRITICAL
					Laser : 1,				//CRITICAL
					VehicleControl : 1,		//NONCRITICAL(assumed) (?)
					GPS : 1,				//NONCRITICAL
					OpenGL : 1,				//(?)
					Camera : 1,				//CRITICAL
					Garbage : 2;
};

union ExecFlags
{
	UnitFlags Flags;
	unsigned char Status;
};

struct ProcessManagement
{
	ExecFlags Heartbeat;
	ExecFlags Shutdown;
	long int LifeCounter;
};

#define NONCRITICALMASK 0xff	//0 011 0000
#define CRITICALMASK 0x0		//0 100 1111
#endif
