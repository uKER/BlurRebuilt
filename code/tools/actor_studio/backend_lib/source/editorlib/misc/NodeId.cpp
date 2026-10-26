#include "stdafx.h"
#include <EditorLib/Misc/NodeId.h>

namespace EditorLib
{

C_NodeId::C_NodeId(const char* str)
{
	FromString(str);
}

void C_NodeId::FromString(const char* str)
{
	Btl::String tmp(str);
	FromString(tmp);
}

void C_NodeId::FromString(const Btl::String& str)
{
	Btl::Vector<Btl::String> strings;
	str.Tokenise(strings, '|');
	if (strings.Size() != 2)
	{
		assert(!"Invalid id created!");
		m_xtId = kInvalidXtId;
		m_childId = kNoChildId;
		return;
	}
	m_xtId = Xt::ParseId(strings[0].Chars());
	m_childId = (U32)strtoul(strings[1].Chars(), 0, 16);

	m_idString = str;
}

const Btl::String& C_NodeId::AsString() const
{
	if (m_idStringInvalid)
	{
		m_idString = Btl::String::Format("%08x|%08x", m_xtId, m_childId);
		m_idStringInvalid = false;
	}

	return m_idString;
}

XtId C_NodeId::GetXtId() const
{
	return m_xtId;
}

U32 C_NodeId::GetChildId() const
{
	return m_childId;
}

bool C_NodeId::IsValid() const
{
	return m_xtId != kInvalidXtId;
}

}
