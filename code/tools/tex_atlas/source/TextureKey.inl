//#include "stdafx.h"
#include <tex_atlas/TextureKey.h>

using namespace TexAtlas;

TextureKey::TextureKey()
{
	Clear();
}

TextureKey::TextureKey(const TextureKey& rhs)
{
	*this = rhs;
}


bool TextureKey::operator ==(const TextureKey& rhs) const
{
	if (
		(m_layerIndex == rhs.m_layerIndex)&&
		(!strcmp(m_fileName.c_str(), rhs.m_fileName.c_str())) &&
		(!memcmp(&m_flags, &rhs.m_flags, sizeof(m_flags))) &&
		(m_uRepeats == rhs.m_uRepeats) &&
		(m_vRepeats == rhs.m_vRepeats))
		return true;
	
	return false;
}

const TextureKey& TextureKey::operator =(const TextureKey& rhs)
{
	m_baseIndex = rhs.m_baseIndex;
	m_layerIndex = rhs.m_layerIndex;
	m_fileName = rhs.m_fileName;
	m_flags = rhs.m_flags;
	m_uRepeats = rhs.m_uRepeats;
	m_vRepeats = rhs.m_vRepeats;
	return *this;
}

void TextureKey::Clear()
{
	m_baseIndex = -1;
	m_layerIndex = -1;
	memset(&m_flags, 0, sizeof(m_flags));
	m_uRepeats = 1.f;
	m_vRepeats = 1.f;
}

void TextureKey::SetFileName(const std::string fileName)
{
	m_fileName = fileName;
}

const std::string& TextureKey::FileName() const
{
	return m_fileName;
}

std::string& TextureKey::FileName() 
{
	return m_fileName;
}

void TextureKey::SetIsBumpMap(bool flag)
{
	m_flags.isBumpMap = flag;
}

bool TextureKey::IsBumpMap() const
{
	return m_flags.isBumpMap;
}

bool TextureKey::operator < (const TextureKey& rhs) const
{
	U32 flags = (U32&)m_flags;
	U32 rhsFlags = (U32&)rhs.m_flags;
	if (
		(m_fileName < rhs.m_fileName) &&
		(!(flags > rhsFlags)) &&
		(!(m_layerIndex > rhs.m_layerIndex)) &&
		(!(m_uRepeats > rhs.m_uRepeats)) &&
		(!(m_vRepeats > rhs.m_vRepeats))
		)
	{
		return true;
	}

	return false;
}

S32 TextureKey::BaseIndex() const
{
	return m_baseIndex;
}

S32 TextureKey::LayerIndex() const
{
	return m_layerIndex;
}

U32 TextureKey::Flags() const
{
	return (U32&)m_flags;
}

void TextureKey::SetBaseIndex(S32 index)
{
	m_baseIndex = index;
}

void TextureKey::SetLayerIndex(S32 index)
{
	m_layerIndex = index;
}

void TextureKey::SetFlags(U32 flags)
{
	m_flags = (FlagBitfield&)flags;
}

void TextureKey::Deserialise(Tu::GenericDiskReader& reader)
{
	reader >> m_fileName;
	{
		U32 flags;
		reader >> flags;
		SetFlags(flags);
	}
	reader >> m_layerIndex;
	reader >> m_baseIndex;
	reader >> m_uRepeats;
	reader >> m_vRepeats;
}

void TextureKey::Serialise(Tu::GenericDiskWriter& writer) const
{
	writer << m_fileName;
	writer << (U32&)m_flags;
	writer << m_layerIndex;
	writer << m_baseIndex;
	writer << m_uRepeats;
	writer << m_vRepeats;
}

F32 TextureKey::URepeats() const
{
	return m_uRepeats;
}

F32 TextureKey::VRepeats() const
{
	return m_vRepeats;
}

 void TextureKey::SetURepeats(F32 repeats)
{
	m_uRepeats = repeats;
}

 void TextureKey::SetVRepeats(F32 repeats)
{
	m_vRepeats = repeats;
}