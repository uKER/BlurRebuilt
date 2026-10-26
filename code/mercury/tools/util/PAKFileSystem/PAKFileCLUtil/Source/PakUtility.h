

#ifndef C_PAKUTILITY_H
#define C_PAKUTILITY_H

#ifdef _MSC_VER
#pragma once
#endif

#include "CommandLineOptions.h"

using namespace Tools;

//-------------------------------------------------------------------
// Class : C_PakUtility
//-------------------------------------------------------------------
class C_PAKUtility : public C_ToolsApplication
{
public:
	C_PAKUtility();

	S32 Main(S32 argc, char **argv, Progress::I_ProgressBar *pProgress);

	Tu::Status ParseCommandLineOptions
	(
		int argc, 
		char* argv[],
		CommandLineOptions &options
	);
};


#endif // C_PAKUTILITY_H