#include "StdAfx.h"
#include "Main.h"
#include "ShaderConstantsCompiler.h"

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 2)
	{
		DBError("Expecting more than one command line argument\n");
		return 1;
	}

	string iFileName(argv[1]);
	string oFileName;

	if (iFileName.substr(iFileName.length() - 4, 4) == ".txt")
	{
		oFileName = iFileName.substr(0, iFileName.length() - 4) + ".h";
	}
	else
	if (iFileName.substr(iFileName.length() - 3, 3) == ".cv" || iFileName.substr(iFileName.length() - 3, 3) == ".cp")
	{
		oFileName = iFileName.substr(0, iFileName.length() - 3) + ".h";
	}
	else
	if (iFileName.substr(iFileName.length() - 3, 3) == ".sv" || iFileName.substr(iFileName.length() - 3, 3) == ".sp")
	{
		oFileName = iFileName.substr(0, iFileName.length() - 3) + ".h";
	}
	else
	{
		DBError("Unsupported file extension %s\n", iFileName.c_str());
		return 1;
	}

	string filePath = iFileName.substr(0, iFileName.find_last_of("\\") + 1);

	C_ShaderConstantsCompiler compiler(filePath);

	ofstream output(oFileName.c_str());

	compiler.Compile(iFileName.c_str(), output);

	return 0;
}
