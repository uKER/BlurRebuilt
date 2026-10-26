#ifndef UBERCOPY_APPLICATION_INCLUDE
#define UBERCOPY_APPLICATION_INCLUDE

#include "tools_shared/ToolsShared.h"
#include "tools_shared/file_copy/FileCopy.h"

class UberCopyApp : public Tools::C_ToolsApplication
{
public:
	UberCopyApp()
		: Tools::C_ToolsApplication(
			"ubercopy",
#if defined(FILE_COPY_XBDM)
			"Bizarre Creations File Copying Tool (xbdm)",
#elif defined(FILE_COPY_XDEVKIT)
			"Bizarre Creations File Copying Tool (xdevkit)",
#else
			"Bizarre Creations File Copying Tool (xbcp)",
#endif
			"tools.filecopy",
			"")
	{
	}

	int Main(int argc, char **argv, Progress::I_ProgressBar *pProgress);
};


#endif