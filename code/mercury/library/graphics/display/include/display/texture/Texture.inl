#ifndef DISPLAY_TEXTURE_TEXTURE_INL
#define DISPLAY_TEXTURE_TEXTURE_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

INLINE S32 C_RefTexture::RefCount() const
{
	return m_RefCount;
}

INLINE const Util::C_String& C_RefTexture::GetFileName() const
{
	return m_FileName;
}

}; // namespace Display

#endif // DISPLAY_TEXTURE_TEXTURE_INL
