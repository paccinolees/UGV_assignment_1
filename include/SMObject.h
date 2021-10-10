#ifndef SMLASER_H
#define SMLASER_H

#include <Windows.h>
#include <tchar.h>
#include "SMfcn.h"

class SMObject
{
	HANDLE CreateHandle;
	HANDLE AccessHandle;
	TCHAR *szName;
	int Size;
public:
	void *pData; //pointer used to point at each shared memories when it is created and provided access, that is of no-type atm. We need to assign its type when we wanna use it
	int SMCreateError;
	int SMAccessError;
public:
	SMObject();
	SMObject(TCHAR* szname, int size);
	~SMObject();
	int SMCreate();
	int SMAccess();
	void SMObject::SetSzname(TCHAR* szname);
	void SMObject::SetSize(int size);
};
#endif


