#ifndef PCFX_H
#define PCFX_H

#ifdef DLL_EXPORT
#define PCFX_API __declspec(dllexport)
#else
#define PCFX_API __declspec(dllimport)
#endif

namespace ActTools
{
	class Scene;
	class Mesh;
}

class apkFile;

#include "fx/BaseFx_Export.h"

class NglPcFx : public NglBaseFx
{
public:
	const char* GetPlatform() const { return "pc"; }
	const char* GetName() const { return "pcfx"; }
	bool IsBigEndian() const { return false; }

	PCFX_API ACTRESULT SerialOutFx( Sys::I_BinaryStream *pStream, const char* fxname, const char** ppszDefines, const char **ppszIncludesconst, const char *pszOutputFilename );

	static NglPcFx* Create() { return new NglPcFx; }
};

#endif // PCFX_H

