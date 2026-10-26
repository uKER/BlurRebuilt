#include "stdafx.h"

#include "lux_common\AlienBrain.h"

#ifndef AMD64
#include <NxNIntegratorSDK.h>
#include <NxNSmartIntegrator.h>

void Log(char*,...);

// initialize alienbrain
AlienBrain::AlienBrain()
{
	m_pIntegrator = new CNxNSmartIntegrator();

	m_pIntegrator->InitInstance(FALSE);
}

// get the latest version of a particular file
bool AlienBrain::GetLatest(const char *psz)
{
	CNxNString nxnpath;

	if (!m_pIntegrator->MapManagedPath(psz, nxnpath))
		return true; // don't need to get latest on local file

	CNxNFile *pFile = (CNxNFile*)m_pIntegrator->GetManagedNode(nxnpath, CNxNFile::GetType());

	if (!pFile)
		return false;

	Log("Getting latest %s\n",psz);

	return pFile->GetLatest(false, false, false, false, false, "", true, false);
}

// check out a file
bool AlienBrain::CheckOut(const char *psz)
{
	CNxNString nxnpath;

	if (!m_pIntegrator->MapManagedPath(psz, nxnpath))
		return false;

	CNxNFile *pFile = (CNxNFile*)m_pIntegrator->GetManagedNode(nxnpath, CNxNFile::GetType());

	if (!pFile)
		return false;

	Log("Checking out %s\n",psz);

	return pFile->CheckOut(false, "Automatically checked out by the lighting tool", false, false, false, "", true);
}

// check a file in
bool AlienBrain::CheckIn(const char *psz)
{
	CNxNString nxnpath;

	if (!m_pIntegrator->MapManagedPath(psz, nxnpath))
		return false;

	CNxNFile *pFile = (CNxNFile*)m_pIntegrator->GetManagedNode(nxnpath, CNxNFile::GetType());

	if (!pFile)
	{
		printf("File %s needs importing into the alienbrain database.\n",nxnpath);
		return false;
	}

	Log("Checking in %s\n",psz);

	return pFile->CheckIn(false,"","Automatically updated by the lighting tool", false, false, false, false, NXN_CHECKIN_CREATE_VERSION_DEFAULT, true);
}


// release alienbrain
AlienBrain::~AlienBrain()
{
	m_pIntegrator->ExitInstance();
	delete m_pIntegrator;
}


#else

// dummy functions for AMD64. need to move these into a separate .exe and run them from here

// initialize alienbrain
AlienBrain::AlienBrain()
{
}

// get the latest version of a particular file
bool AlienBrain::GetLatest(const char *psz)
{
	return false;
}

// check out a file
bool AlienBrain::CheckOut(const char *psz)
{
	return false;
}

// check a file in
bool AlienBrain::CheckIn(const char *psz)
{
	return false;
}


// release alienbrain
AlienBrain::~AlienBrain()
{
}


#endif