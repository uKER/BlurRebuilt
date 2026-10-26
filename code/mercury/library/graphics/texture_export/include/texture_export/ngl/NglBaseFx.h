#ifndef NGLBASEFX_H
#define NGLBASEFX_H

#include "NglBaseDriver.h"

namespace ActTools
{
	class Scene;
	class Mesh;
}

class apkFile;

class NglBaseFx : public NglBaseDriver
{
public:
	virtual ACTRESULT SerialOutFx( Sys::I_BinaryStream *pStream, const char* fxname, const char* flags = "" ) = 0;

};

#endif // NGLBASEFX_H

