#include "SystemPch.h"
#include "IoImpl.h"
#include <System/Serialise/C_FileSystem.h>

const char* g_ioReadTypeStr[kIoReadType_Count] =
{
	"unknown",
	"buffered",
	"unbuffered",
	"speculative"
};

#if 0
void DoOpen()
{
	SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_ABOVE_NORMAL);


	IoInitialise();

	int bigBuffer = 1;

	char* bob = new char[bigBuffer];

	for(int i = 0; i < 20; i++)
	{
		Sys::I_File* a = 0;
		const char* type = "";

		if (i%2)
		{
			AsyncFile* f;
			a = f = new AsyncFile();
			f->Open("GAME:\\test2.bin");
			type = "Async";
		}
		else
		{
			a = Sys::C_FileSystem::Instance().Open("GAME:\\test2.bin", Sys::E_FileAccess_Read);
			type = "CFile";
		}

		DWORD start = GetTickCount();

		int yay = 0;
		
		while(a->Read(bob, bigBuffer))
			yay += bigBuffer;

		DebugLn("%s %i (%i)", type, GetTickCount() - start, yay);

		delete a;
	}

	delete [] bob;

	DebugLn("Shutdown");

	IoShutdown();
}

ArteryThread gAsyncTest;

void AsyncTest2(void*)
{
	DoOpen();
}
#endif 

void AsyncTest()
{
	//gAsyncTest.Create(AsyncTest2, 0, 0);
}