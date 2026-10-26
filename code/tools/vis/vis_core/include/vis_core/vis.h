#pragma once

#include "vis_core/ReadXMLConfig.h"

namespace Progress
{
class I_ProgressBar;
};

class SampleAreaList;
class BitfieldDatabase;

#define MAX_SAMPLES 256

#define VISLIST_NOTHING 0
#define VISLIST_COMPARE 1
#define VISLIST_CREATE 2

class Vis
{
public:
			Vis();
			~Vis();

	void	(*m_pLogCallbackFunc)(const std::string& text);

	bool	Process(Progress::I_ProgressBar *pProgress);
	void	LoadXML(const std::string filename);
	void	Save(const char *psz);
	bool	TestScreenGrabbing();
	void	DisplayCell(int i);
	void	DisplayUsage();
	void	LoadDatabase(const std::string filename);
	void	DebugCode();
	void	SetScene(Aftk::C_Scene *pScene);
	void	SetGUIMode(bool bGuiOn);
	void	SetVS2(const std::string filename);
	void	SetRemoteClientPath(const std::string &path);
	void	SetVisListPath(const std::string &path, int iMode);
	void	SetTransparentPVSPolyFlagMask(U32 iMask);
	void	OverrideSamplePoints(const std::string &str);
	void	AutoGenSamplePointsForCar();
	bool	SetXMLConfigFile(const char *pszfilename);
	void	SetClusterThreshold(int i);
	void	SetPolygonThreshold(int i);
	void	SetPaths(const char *pszTexCachePath, const char *pszFXBPath);
static void Vis::JobCompletedCallbackFunction(void *pThis, JobScheduler *pScheduler, JobScheduler::Job *pJob);

	BitfieldDatabase	*m_pDatabase;
	Aftk::C_Scene			*m_pScene;

	SampleAreaList		*m_pSampleAreas;
	Ut::Vector<BYTE>		*m_pVS2Data;
	bool				m_bGuiMode;
	bool				m_bUseNetworkClients;
	std::string			m_sNetworkClientsRemotePath;

	Ut::Vector<int>		m_visList;
	std::string			m_visListPath;
	std::string			m_sFXBPath, m_sTextureCachePath;
	int					m_iVisListMode;
	int					m_iTransparentPVSPolyFlagMask;
	VisConfig			m_Config;
	int					m_iClusterThreshold;
	int					m_iPolygonThreshold;
};

extern bool gbDebugMode;
extern int gnTotalSamples;
