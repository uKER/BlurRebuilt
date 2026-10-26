#include "StdAfx.h"
#include "Main.h"

#if 0

void M00(const U32 m00)
{
	CORE_PRINT("I AM M00 %i\n", m00);
}

int main(int argc, char *argv[])
{
	U8* pM001 = (U8*)M00;
	CORE_PRINT("0x%.8x\n", pM001); // Wrong address...
	
	U8* pM002 = *((U8**)M00);
	CORE_PRINT("0x%.8x\n", pM002); // Correct address...

	void (*handler)(const uint32_t head) = (void (*)(std::uint32_t))pM001;

	(*handler)(0);

	return 0;
}

#else

#include "CoreSystems.h"
#include "Scheduler.h"
#include "Application.h"
#include "Sample.h"
#include "SampleDefault.h"

#if PLATFORM_PS3
namespace PlatformPS3
{
char g_ps3_titleId[ 512 ] = "ABCD12345";
}
#endif

#if  PLATFORM_PS3
SYS_PROCESS_PARAM (1001, 128 * 1024) // (priority, stack size)
#endif

#if PLATFORM_PC
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
#else
int main(int argc, char *argv[])
#endif
{
	bool success;
	
	success = C_CoreSystems::Initialise();
	BIZ_ASSERT(success);

	success = C_Scheduler::Initialise();
	BIZ_ASSERT(success);

	C_ApplicationParams applicationParams;

	#if PLATFORM_PC
	applicationParams.m_hInstance = hInstance;
	#endif

	C_Application application;
	success = application.Create(applicationParams);
	BIZ_ASSERT(success);

	Display::C_Device& display = application.m_Device;
	Display2::C_Device& display2 = display.m_Display2;

#if 1
	C_Sample* pSample = new C_SampleDefault(&application);
	success = pSample->Create();
	BIZ_ASSERT(success);
	success = pSample->CreateVolatile();
	BIZ_ASSERT(success);
	application.SetSample(pSample);
#else
	C_Sample* pSample = NULL;
#endif

	//display2.DumpVertexBuffers();
	//display2.DumpIndexBuffers();
	//display2.DumpVertexShaders();
	//display2.DumpPixelShaders();
	//display2.DumpSurfaces();
	//display2.DumpTextures();

	application.Run();

	if(pSample)
	{
		pSample->DestroyVolatile();
		pSample->Destroy();

		SAFE_DELETE_PTR(pSample);
	}

	application.Destroy();

	C_Scheduler::Shutdown();

	C_CoreSystems::Shutdown();
	
	return 0;
}

#endif
