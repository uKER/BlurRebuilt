#ifndef PS3FX_H
#define PS3FX_H

#ifdef DLL_EXPORT
#define PS3FX_API __declspec(dllexport)
#else
#define PS3FX_API __declspec(dllimport)
#endif

namespace ActTools
{
	class Scene;
	class Mesh;
}

class apkFile;

#include "fx/BaseFx_Export.h"

class NglPs3Fx : public NglBaseFx
{
public:
	virtual const char* GetPlatform() const { return "ps3"; }
	virtual const char* GetName() const { return "ps3mesh"; }
	virtual bool IsBigEndian() const { return true; }

	PS3FX_API ACTRESULT SerialOutFx( Sys::I_BinaryStream *pStream, const char* fxname, const char** ppszDefines, const char **ppszIncludes, const char *pszOutputFilename );

	static NglPs3Fx* Create() { return new NglPs3Fx; }
};

#endif // PS3FX_H

