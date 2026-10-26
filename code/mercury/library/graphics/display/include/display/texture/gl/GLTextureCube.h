#ifndef DISPLAY_GLTEXTURECUBE_H
#define DISPLAY_GLTEXTURECUBE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Texture/GL/GLTexture.h"
#include "Display/Texture/TextureParams.h"

namespace NewDisplay
{
namespace GL
{

class C_TextureCube : public C_Texture
{
public:

	C_TextureCube();
	~C_TextureCube();

	const bool IsCreated() const;

	const U32 GetSize();
	U8* GetAddress();
	const Util::C_String GetFileName();
	const U64 GetFileTime();
	const S32 GetSystemID();

	void SetSize(const U32 size);
	void SetAddress(U8* pAddress);
	void SetFileName(const char* fileName);
	void SetFileTime(const U64 fileTime);
	void SetSystemID(const S32 systemID);

	bool Upload(C_TextureParams* pParams);

	void* Lock(const E_CubeTextureFace face, const U32 level);
	void Unlock(const E_CubeTextureFace face, const U32 level);
};

#include "GLTextureCube.inl"

}; // namespace GL
}; // namespace NewDisplay

#endif // DISPLAY_GLTEXTURECUBE_H
