#include "StdAfx.h"
#include "ShaderProjectCompiler.h"

int main(int argc, char* argv[])
{
	int errorCode = ShaderProjectCompiler(argc, argv);

	//char c = _getch();

	return errorCode;
}
