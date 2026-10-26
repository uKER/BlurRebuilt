#include "StdAfx.h"
#include "Main.h"

using namespace std;

int ViewFile(const string& file)
{
	FILE* pFile = NULL;
	int error = fopen_s(&pFile, file.c_str(), "rb+");
	if (error == 0)
	{
		fseek(pFile, 0, SEEK_END);
		long fileSize = ftell(pFile);
		fseek(pFile, 0, SEEK_SET);
		if (fileSize > 0)
		{
			BYTE* pData = new BYTE[fileSize];
			memset(pData, 0, fileSize);

			fread(pData, 1, fileSize, pFile);

			int* pInts = (int*)pData;

			delete[] pData;
		}

		fclose(pFile);
		return 0;
	}
	else
	{
		cerr << "Failed to open file " << file.c_str() << endl;
		return 1;
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	string file = "vsFontRender.p3v";

	int exitCode = ViewFile(file);
	
	cout << "Press any key to continue...";
	char c = _getch();

	return exitCode;
}
