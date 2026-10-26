#include "stdafx.h"

#include <EditorLib/Xt/XtDirty.h>
#include <EditorLib/Xt/XtHelpers.h>

namespace EditorLib
{

C_XtDirty::C_XtDirty()
{
	m_allDirty = false;
}

bool C_XtDirty::IsDirty() const
{
	return m_allDirty || m_dirtyElements.Size() != 0;
}

bool C_XtDirty::IsDirty(const Xt::DomElement* pElement) const
{
	if (m_allDirty) 
		return true;

	if (!pElement)
		return false;

	for (Btl::OpenHashSet<const Xt::DomElement*, Btl::HashKeyOpPtr>::ConstIterator it = m_dirtyElements.Begin(); it != m_dirtyElements.End(); ++it)
	{
		if (*it && XtHelpers::AreRelated(pElement, *it))
			return true;
	}
	return false;
}

void C_XtDirty::SetDirty(const Xt::DomElement* pElement)
{
	m_dirtyElements.Insert(pElement);
}

void C_XtDirty::SetDirty()
{
	m_allDirty = true;
}

void C_XtDirty::Clear()
{
	m_dirtyElements.Clear();
	m_allDirty = false;
}

}