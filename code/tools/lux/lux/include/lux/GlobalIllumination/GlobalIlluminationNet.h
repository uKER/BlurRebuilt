#ifndef GLOBALILLUMINATION_NET
#define GLOBALILLUMINATION_NET

class JobServer;
namespace ost
{
	class Thread;
}
namespace Progress
{
	class I_ProgressBar;
};

struct RunServerStruct
{
	JobServer		*pServer;
	ost::Thread		*pClient;
	bool			bUseNetworkClients;
	std::string		clientExePath;	// for windows
	std::string		clientElfPath;	// for G5
	char			pszTmpFilename1[512];
	char			pszTmpFilename2[512];
	LUX::Lux		*pLux;
};

/// take care of the starting of the server, displaying a progress indicator and returns when all jobs have been rendered
void InitServerPaths(RunServerStruct &net);
bool RunServer(RunServerStruct &net, Progress::I_ProgressBar *pProgress);
void DeleteServerPaths(RunServerStruct &net);

#endif
