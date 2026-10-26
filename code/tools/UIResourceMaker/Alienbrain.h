#pragma once

#include "NxNIntegratorSDK.h"
#include "NxNSmartIntegrator.h"

const int maxFiles=200;

class AlienBrain
{
public:
	AlienBrain();
	~AlienBrain();

	bool checkOutFile(int index);
	bool checkInFile(int index);
	bool getLatest();

	int registerFile(const char* fileName);

	CNxNString alienPath[maxFiles], *path[maxFiles];

	CNxNSmartIntegrator* m_pIntegrator;
	
	CNxNFile   *m_pFile[maxFiles];

	bool fileActive[maxFiles];
	bool fileCheckedOut[maxFiles];
};

extern AlienBrain gAlienBrain;
