#ifndef APPLICATION_H
#define APPLICATION_H

#ifdef _MSC_VER
#pragma once
#endif

/// base class for tools applications
class C_Application : public Tools::C_ToolsApplication
{
public:
	C_Application(
		const char *szName,			// the name of the application
		const char *szSubtitle,		// subtitle e.g. "bizarre creations lighting tool"
		const char *szLogger,		// name of the base logger for this application
		const char *szDmpPath,		// path where .dmp files are to be written out to
		bool bExecuter = false		// true if the application needs to send executer feedback
		);

	/// destructor
	virtual ~C_Application();

public:
	/// main callback function to be overloaded by derived class
	virtual	int Main(S32 argc, char **argv, Progress::I_ProgressBar *pProgress);

	bool ParseCommandLineOptions(
		int argc, 
		char* argv[], 
		C_Options& options);
};

#endif
