#ifndef AFTK_CONV_APPLICATION_H
#define AFTK_CONV_APPLICATION_H

#ifdef _MSC_VER
#pragma once
#endif

#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.VC90.CRT' version='9.0.21022.8' processorArchitecture='X86' publicKeyToken='1fc8b3b9a1e18e3b' language='*'\"")

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

private:
	/// main callback function to be overloaded by derived class
	virtual	int Main(S32 argc, char **argv, Progress::I_ProgressBar *pProgress);

	bool ParseCommandLineOptions(
		int argc, 
		char* argv[], 
		C_Options& options);

	E_ReturnCode Process(C_Options& options);
};

#endif
