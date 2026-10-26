#ifndef NGLBASEFX_H
#define NGLBASEFX_H

namespace ActTools
{
	class Scene;
	class Mesh;
}

class apkFile;

class NglBaseFx 
{
public:
	virtual ACTRESULT SerialOutFx( Sys::I_BinaryStream *pStream, const char* fxname, const char** ppszDefines, const char **ppszIncludes, const char *pszOutputFilename ) = 0;

};

#endif // NGLBASEFX_H

