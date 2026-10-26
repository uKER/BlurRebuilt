#include "stdafx.h"
#include "alienbrain.h"
#include "commandline.h"

AlienBrain gAlienBrain;

AlienBrain::AlienBrain()
{

//#ifndef _DEBUG
	//m_pIntegrator = new CNxNSmartIntegrator();

//	if(m_pIntegrator)
//	{
//		if(!(m_pIntegrator->InitInstance(FALSE)))
//		{
//			gCommandLine.AddLine(TC_RED,"Alienbrain Error - Unable to Initialise");
//			return;
//		}
//	}
//#endif

	for (int i=0; i<maxFiles; i++)
	{
		path[i]=0;
		m_pFile[i] =0;

		fileActive[i] = false;
		fileCheckedOut[i] = false;
	}
}

AlienBrain::~AlienBrain()
{
	if (m_pIntegrator)
	{
		for(int i=0; i<maxFiles; i++)
		{
			if (path[i])
			{
				delete path[i];
			}
		}
		
		if(!m_pIntegrator->ExitInstance()) gCommandLine.AddLine(TC_RED,"AB ExitInstance error");

		delete m_pIntegrator;
	}
}

int AlienBrain::registerFile(const char* fileName)
{
	if(!m_pIntegrator) return 0;

	int i=0;
	while(fileActive[i]){i++;}
	if (i >= maxFiles)
	{
		gCommandLine.AddLine(TC_RED,"Too many files checked out");
		return maxFiles;
	}

	path[i] = new CNxNString(fileName);
	if (!(m_pIntegrator->MapManagedPath(*path[i], alienPath[i])))
	{
		gCommandLine.AddLine(TC_RED,"Cannot Map Path");
	}

	fileActive[i] = true;

	if ((fileName[0] == 'z') || (fileName[0] == 'Z' ))
	{
		if ((m_pIntegrator->FileCheckedOutByAnotherUser(*path[i])))
		{
			gCommandLine.AddLine(TC_RED,"File checked out by someone else, loading local copy...");
		}
		else
		{
			if(m_pIntegrator->FileCheckedOutByUser(*path[i]) )
			{
				fileCheckedOut[i] = true;

				m_pFile[i] = (CNxNFile*)m_pIntegrator->GetManagedNode(alienPath[i], CNxNFile::GetType());
			}
		}
	}
	return i;

}


bool AlienBrain::checkOutFile(int i)
{
	if(!m_pIntegrator) return 0;

	if (i >= maxFiles)
	{
		gCommandLine.AddLine(TC_RED,"Too many files checked out");
		return false;
	}

	m_pFile[i] = (CNxNFile*)m_pIntegrator->GetManagedNode(alienPath[i], CNxNFile::GetType());
	if (m_pIntegrator->FileCheckedOutByUser(*path[i]))
	{
		fileCheckedOut[i]=true;
		return true;
	}
	else
	{

		if (m_pFile[i] == NULL)
		{
			gCommandLine.AddLine(TC_RED,"File checkout error, filename not recognised by AlienBrain");
			return false;
		}

		if (m_pFile[i]->CheckOut())
		{
			if (m_pFile[i]->IsCheckedOut())
			{
				fileCheckedOut[i] = true;
				return true;
			}
			else
			{
				gCommandLine.AddLine(TC_RED,"File checkout failed as file is not checked out!");
				return false;
			}
		}
		else
		{
			gCommandLine.AddLine(TC_RED,"File checkout failed according to AB SDK");
			return false;
		}
		
	}
}

bool AlienBrain::checkInFile(int i)
{
	if(!m_pIntegrator) return 0;

	if (i >= maxFiles)
	{
		gCommandLine.AddLine(TC_RED,"Too many files checked out");
		return false;
	}

	if(m_pFile[i])
	{
		if (m_pFile[i]->CheckIn())
		{
			fileCheckedOut[i] = false;
			return true;
		}
		gCommandLine.AddLine(TC_RED,"File could not be checked in, alienbrain SDK reports error");
		return false;
		
	}
	else
	{
		gCommandLine.AddLine(TC_RED,"File name doesn't exist within editor");
		return false;
	}


}



bool AlienBrain::getLatest()
{


	return true;
}
