#include "stdafx.h"
#include <tex_atlas/InputList.h>

namespace TexAtlas
{
	InputList::InputList(const InputList& rhs)
	{
		*this = rhs;
	}

	const InputList& InputList::operator =(const InputList& rhs)
	{
		m_textureList = rhs.m_textureList;
		return *this;
	}

	U32 InputList::AddRecord(const ImageDescription& r)
	{
		m_textureList.PushBack(r);
		return (U32)m_textureList.Size()-1;
	}

	const ImageDescription& InputList::Record(U32 index) const
	{
		return m_textureList[index];
	}

	ImageDescription& InputList::Record(U32 index)
	{
		return m_textureList[index];
	}

	U32 InputList::NumRecords() const
	{
		return (U32)m_textureList.Size();
	}
}
