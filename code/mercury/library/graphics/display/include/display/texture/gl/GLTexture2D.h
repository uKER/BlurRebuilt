#ifndef DISPLAY_GLTEXTURE2D_H
#define DISPLAY_GLTEXTURE2D_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Texture/GL/GLTexture.h"
#include "Display/Texture/TextureParams.h"

namespace NewDisplay
{
namespace GL
{

class C_Texture2D : public C_Texture
{
public:

	C_Texture2D();
	~C_Texture2D();

	const bool IsCreated() const;

	const U32 GetWidth();
	const U32 GetHeight();
	const U32 GetLevels();
	const E_Format GetFormat();
	const U32 GetSize();
	U8* GetAddress();
	const U32 GetFlags();
	const Util::C_String GetFileName();
	const U64 GetFileTime();
	const S32 GetSystemID();

	void SetSize(const U32 size);
	void SetAddress(U8* pAddress);
	void SetFlags(const U32 flags);
	void SetFileName(const char* fileName);
	void SetFileTime(const U64 fileTime);
	void SetSystemID(const S32 systemID);

	bool Upload(C_TextureParams* pParams);

	void* Lock(const U32 level);
	void Unlock(const U32 level);

private:

	const U32 Size(const E_Format format, const U32 width, const U32 height);

private:

	U32 m_Flags;
};

#include "GLTexture2D.inl"

}; // namespace GL
}; // namespace NewDisplay

#endif // DISPLAY_GLTEXTURE2D_H
