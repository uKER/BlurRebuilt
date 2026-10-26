#ifndef DISPLAY_GLTEXTURE3D_H
#define DISPLAY_GLTEXTURE3D_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Texture/GL/GLTexture.h"
#include "Display/Texture/TextureParams.h"

namespace NewDisplay
{
namespace GL
{

class C_Texture3D : public C_Texture
{
public:

	C_Texture3D();
	~C_Texture3D();

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
};

#include "GLTexture3D.inl"

}; // namespace GL
}; // namespace NewDisplay

#endif // DISPLAY_GLTEXTURE3D_H
