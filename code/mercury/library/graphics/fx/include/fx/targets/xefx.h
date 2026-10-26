#ifndef XEFX_H
#define XEFX_H

#ifdef DLL_EXPORT
#define XEFX_API __declspec(dllexport)
#else
#define XEFX_API __declspec(dllimport)
#endif

namespace ActTools
{
	class Scene;
	class Mesh;
}

struct IDirect3DTexture9;
struct IDirect3DBaseTexture9;

class apkFile;

#include "fx/BaseFx_Export.h"

class NglXeFx : public NglBaseFx
{
public:
	virtual const char* GetPlatform() const { return "xenon"; }
	virtual const char* GetName() const { return "xefx"; }
	virtual bool IsBigEndian() const { return true; }

	XEFX_API ACTRESULT SerialOutFx( Sys::I_BinaryStream *pStream, const char* fxname, const char** ppszDefines, const char **ppszIncludes, const char *pszOutputFilename );

	static NglXeFx* Create() { return new NglXeFx; }
};


#endif // XEFX_H

