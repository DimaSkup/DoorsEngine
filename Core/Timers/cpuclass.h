////////////////////////////////////////////////////////////////////
// Filename:     cpuclass.h
// Description:  this class is used to determine the percentage 
//               of total cpu use that is occuring each second
// Revising:     31.07.22
////////////////////////////////////////////////////////////////////
#pragma once

//////////////////////////////////
// LINKING
//////////////////////////////////
#pragma comment(lib, "pdh.lib")


//////////////////////////////////
// INCLUDES
//////////////////////////////////
#include <pdh.h>


//////////////////////////////////
// Class name: CpuClass
//////////////////////////////////
class CpuClass
{
public:
	CpuClass();
	CpuClass(const CpuClass&);
	~CpuClass();

	void Initialize();
	void Shutdown();
	void Update();
	int GetCpuPercentage();

private:
	bool m_canReadCpu = false;
	HQUERY m_queryHandle = NULL;
	HCOUNTER m_counterHandle = 0;
	ULONG m_lastSampleTime = 0;
	LONG m_cpuUsage = 0;;
};
