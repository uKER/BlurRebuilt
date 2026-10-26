#include "stdafx.h"
#include "NativeDefinitions.h"
#include "NativeDiskReader.h"
#include "NativeDiskWriter.h"
#include "NativeUtility.h"

using namespace Aftk;

void C_NativeUtility::EndianSwitch(void* p, U32 nBytes)
{
	static char buffer[16384];

	char* pOut;
	if (nBytes >= 16384)
		pOut = new char[nBytes];
	else
		pOut = buffer;

	// endian switching
	char *pIn = (char*)p;

	int c = nBytes;
	for (int i=0; i<c; i++)
		pOut[i] = pIn[c - i - 1];

	memcpy(p, pOut, nBytes);

	if (nBytes >= 16384)
		delete[] pOut;
}

U32 C_NativeUtility::GetBlockIdInt(const char* id)
{
	U32 idInt;
	memcpy(&idInt, id, sizeof(U32));
	return idInt;
}
