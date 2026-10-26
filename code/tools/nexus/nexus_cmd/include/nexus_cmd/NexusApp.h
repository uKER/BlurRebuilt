
#ifndef NEXUSCMD_NEXUSAPP_H
#define NEXUSCMD_NEXUSAPP_H


namespace NexusCmd
{

class MainApp : public Tools::C_ToolsApplication
{
public:
	MainApp();

	~MainApp();

	void SearchAndReplace
	(
		std::string& theString, 
		const std::string& theSearchSubString, 
		const std::string& theNewSubString
	);

	void DoHeader();

	Tu::Status ParseCommandLineOptions
	(
		int argc, 
		char* argv[], 
		CommandLineOptions& commandLineOptions
	);

	void CleanUpMaya();

	Tu::Status Create(NexusCmd::CommandLineOptions& options);

	void Destroy();

	int Main(int argc, char **argv, Progress::I_ProgressBar *pProgress);
};

}// namespace NexusCmd

#endif // NEXUSCMD_NEXUS_CMD_H