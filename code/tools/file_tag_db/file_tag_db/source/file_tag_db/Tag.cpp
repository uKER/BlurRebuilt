#include "stdafx.h"
#include <file_tag_db/GenericAttribute.h>
#include <file_tag_db/Tag.h>

using namespace FileTag;

U32 C_Tag::GetNumAttributes() const
{
	assert(m_attributes.Size() > -1);
	return (U32)m_attributes.Size();
}

const Util::C_String& C_Tag::GetFileName() const
{
	return m_fileName;
}

Util::C_String& C_Tag::GetFileName()
{
	return m_fileName;
}

Ut::Vector<C_GenericAttribute>& C_Tag::GetAttributes()
{
	return m_attributes;
}

const Ut::Vector<C_GenericAttribute>& C_Tag::GetAttributes() const
{
	return m_attributes;
}

const C_GenericAttribute* C_Tag::GetAttributePtr(const Util::C_String &name) const
{
	return const_cast<C_Tag*>(this)->GetAttributePtr(name);
}

C_GenericAttribute* C_Tag::GetAttributePtr(const Util::C_String &name)
{
	// look through attributes of this entry to find the right one
	for (S32 j = 0; j < m_attributes.Size(); j++)
	{
		C_GenericAttribute& gAttr = m_attributes[j];
		if (gAttr.GetName() == name)
			return &gAttr;
	}
	// not found
	return false;
}