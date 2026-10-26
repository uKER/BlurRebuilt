#include "stdafx.h"
#include <dialogueman/misc/DomId.h>

namespace DMan
{

void C_DomId::FromString(const Btl::String& str)
{
	Btl::Vector<Btl::String> strings;
	str.Tokenise(strings, '|');
	if (strings.Size() != 3)
	{
		assert(!"Invalid id created!");
		m_packId = kInvalidXtId;
		m_recordId = kInvalidXtId;
		return;
	}
	m_packId = (U32)strtoul(strings[0].Chars(), 0, 16);
	m_recordId = (U32)strtoul(strings[1].Chars(), 0, 16);
	m_recordId = (S32)strtol(strings[2].Chars(), 0, 16);
}

Btl::String C_DomId::AsString() const
{
	return Btl::String().Printf("%x|%x", m_packId, m_recordId);
}

XtId C_DomId::GetPackId() const
{
	return m_packId;
}

XtId C_DomId::GetRecordId() const
{
	return m_recordId;
}

S32 C_DomId::GetChildId() const
{
	return m_childId;
}

bool C_DomId::IsValid() const
{
	return m_packId != kInvalidXtId;
}

}
