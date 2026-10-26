#include "StdAfx.h"
#include "Main.h"

#include <Display2/Context/Context.h>

#if defined(PLATFORM_XENON)

void __cdecl main()
{
}

#elif defined(PLATFORM_PS3)

int main(int argc, char *argv[])
{
	U32 m00 = Display2::E_TextureGamma_R | Display2::E_TextureGamma_B | Display2::E_TextureGamma_B | Display2::E_TextureGamma_A;
	U32 m001 = Display2::E_TextureGamma_R | Display2::E_TextureGamma_B | Display2::E_TextureGamma_A;
	U32 m003 = Display2::E_TextureGamma_R | Display2::E_TextureGamma_A;
	
	return 0;
}

#else

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	return 0;
}

#endif
