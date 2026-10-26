#include "StdAfx.h"
#include "ShaderCompiler.h"

int _tmain(int argc, _TCHAR* argv[])
{
	ShaderCompiler::C_ShaderCompiler shaderCompiler;
	
	bool success = shaderCompiler.Compile(argc, argv);

	//char c = _getch();

	return success ? 0 : 1;
}
