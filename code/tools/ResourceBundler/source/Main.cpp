// This is the main project file for VC++ application project 
// generated using an Application Wizard.

#include "stdafx.h"

#include "BundlerOutputOptions.h"
#include "Bundler.h"

using namespace ResourceBundler;

#ifndef WIN64
#pragma comment(linker, "\"/manifestdependency:type='Win32' name='Microsoft.VC90.CRT' version='9.0.21022.8' processorArchitecture='X86' publicKeyToken='1fc8b3b9a1e18e3b' language='*'\"")
#endif 

int main(int argc, char *argv[])
{
	const char* name = "Bizarre Creations Resource Bundler v2.0";
	C_Bundler bundle(name, "", "", "", false);

#ifdef _DEBUG
	printf( "*** DEBUG ***\n\nWaiting for Debugger to Connect\n" );
	MessageBox( 0, "Attach debugger to ResourceBuilder.", "ResourceBuilder Debug", 0 );
#endif

	U32 start = GetTickCount();
	int exitCode = bundle.Run(argc, argv);
	U32 end = GetTickCount();

	U32 delta = end - start;
	printf("\nProcessing %s after %.2f seconds\n", 
		exitCode == 0 ? "succeeded" : "failed",
		(F32)(delta) / 1000.f);

	return exitCode;
}

