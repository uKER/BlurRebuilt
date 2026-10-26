#ifndef PS3TEX_H
#define PS3TEX_H

class asdVariant;

namespace ActTools
{
	class Texture;
	class TexelFormat;
}

class apkFile;
class apkFileEntry;

#include "texture_export/ngl/NglBaseTex.h"

class NglPs3Tex : public NglBaseTex
{
public:
	const char* GetPlatform() const { return "ps3"; }
	const char* GetName() const { return "ps3tex"; }
	bool IsBigEndian() const { return true; }

	ACTRESULT SerialOutTexture( Sys::I_BinaryStream* stream, const ActTools::Texture* aTexture, OutputType outputType = HEADER_AND_DATA );
	ACTRESULT SerialInTexture( ActTools::Texture* aTexture, Sys::I_BinaryStream* stream );
	asdVariant* SerialInTextureStats(const char* aFileName);

	static NglPs3Tex* Create() { NglPs3Tex* tex = new NglPs3Tex; return tex; }

	// Get TexelFormat class object from GPU format.
	ActTools::TexelFormat* GetTexelFormat( int format );
	ActTools::TexelFormat* GetTexelFormat( const char* format );
};

#endif // PS3TEX_H

