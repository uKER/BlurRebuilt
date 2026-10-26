// This is the main project file for VC++ application project 
// generated using an Application Wizard.

#include "stdafx.h"

#include "BundlerOutputOptions.h"
#include "Bundler.h"

using namespace ResourceBundler;

int main(int argc, char *argv[])
{
	const char* name = "Bizarre Creations Resource Bundler v2.0";
	C_Bundler bundle(name, "", "", "", false);

	U32 start = GetTickCount();
	int exitCode = bundle.Run(argc, argv);
	U32 end = GetTickCount();

	U32 delta = end - start;
	printf("\nProcessing %s after %.2f seconds\n", 
		exitCode == 0 ? "succeeded" : "failed",
		(F32)(delta) / 1000.f);

	return exitCode;
}

