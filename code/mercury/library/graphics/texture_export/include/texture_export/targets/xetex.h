#ifndef XETEX_H
#define XETEX_H

class asdVariant;

namespace ActTools
{
	class Texture;
	class TexelFormat;
}

class apkFile;
class apkFileEntry;

#include "texture_export/ngl/NglBaseTex.h"

#define NGL_XETEX_CACHED_MEMORY  1
#define NGL_XETEX_LARGE_PAGES    2

class NglXeTex : public NglBaseTex
{
public:
	const char* GetPlatform() const { return "xenon"; }
	const char* GetName() const { return "xetex"; }
	bool IsBigEndian() const { return true; }

	ACTRESULT SerialOutTexture( Sys::I_BinaryStream* stream, const ActTools::Texture* aTexture, OutputType outputType = HEADER_AND_DATA );
	ACTRESULT SerialInTexture( ActTools::Texture* aTexture, Sys::I_BinaryStream* stream );

	static NglXeTex* Create() { return new NglXeTex; }

	size_t GetSurfaceBytes( int format, size_t w, size_t h );

	// Convert from D3DFORMAT <-> string.
	int GetD3DFormat( const char* id );
	const char* GetD3DFormatName( int format );

	// Get TexelFormat class object from D3DFORMAT.
	ActTools::TexelFormat* GetTexelFormat( int format );
	ActTools::TexelFormat* GetTexelFormat( const char* format );

	void SetFlags(int _Flags)
	{
		Flags = _Flags;
	}

	static int Flags;
};


#endif // XETEX_H

