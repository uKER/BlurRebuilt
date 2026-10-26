#ifndef PCTEX_H
#define PCTEX_H

#ifndef SWIG
 #ifdef PCTEX_EXPORT
  #define PCTEX_API //__declspec(dllexport)
 #else
  #define PCTEX_API //__declspec(dllimport)
 #endif // PCTEX_EXPORT
#else
 #define PCTEX_API
#endif //SWIG

class asdVariant;

namespace ActTools
{
	class Texture;
	class TexelFormat;
}

class apkFile;
class apkFileEntry;

#include "texture_export/ngl/NglBaseTex.h"

class PCTEX_API NglPcTex : public NglBaseTex
{
public:
	const char* GetPlatform() const { return "pc"; }
	const char* GetName() const { return "pctex"; }
	bool IsBigEndian() const { return false; }

	ACTRESULT SerialOutTexture( Sys::I_BinaryStream* stream, const ActTools::Texture* aTexture, OutputType outputType = HEADER_AND_DATA );
	ACTRESULT SerialInTexture( ActTools::Texture* aTexture, Sys::I_BinaryStream* stream );

	static NglPcTex* Create() { return new NglPcTex; }

	// Calculates the amount of memory used by a surface.
	size_t GetSurfaceBytes( int format, size_t w, size_t h );

	// Convert from D3DFORMAT <-> string.
	int GetD3DFormat( const char* id );
	const char* GetD3DFormatName( int format );

	// Get TexelFormat class object from D3DFORMAT.
	ActTools::TexelFormat* GetTexelFormat( int format );
	ActTools::TexelFormat* GetTexelFormat( const char* format );
};

#endif // PCTEX_H

