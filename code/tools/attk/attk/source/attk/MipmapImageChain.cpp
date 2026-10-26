#include "stdafx.h"

#include <attk/Enum.h>
#include <attk/Status.h>
#include <attk/Global.h>
#include <attk/Progress.h>
#include <attk/OptionAttribute.h>
#include <attk/Options.h>
#include <attk/BaseImage.h>
#include <attk/PixelFormats.h>
#include <attk/Image.h>

#include <attk/MipmapImageChain.h>

Attk::C_MipmapImageChain::C_MipmapImageChain()
{
}

Attk::C_MipmapImageChain::~C_MipmapImageChain()
{
	Clear();
}

U32 Attk::C_MipmapImageChain::AddImagePtr(C_Image* pImage)
{
	m_mipLevels.PushBack(pImage);
	return (U32)m_mipLevels.Size();
}

U32 Attk::C_MipmapImageChain::GetNumLevels() const
{
	return (U32)m_mipLevels.Size();
}

Attk::C_Image* Attk::C_MipmapImageChain::GetLevelPtr(U32 mipLevel)
{
	return m_mipLevels[mipLevel];
}

const Attk::C_Image* Attk::C_MipmapImageChain::GetLevelPtr(U32 mipLevel) const
{
	return m_mipLevels[mipLevel];
}

void Attk::C_MipmapImageChain::Clear()
{
	for (S32 i = 0; i < m_mipLevels.Size(); i++)
		delete m_mipLevels[i];
	m_mipLevels.Clear();
}
