#include "stdafx.h"

#include <Xt_Shared/XtDirty.h>

namespace EditorLib
{
	C_XtDirty::C_XtDirty()
	{
		m_deferred = false;
	}

	bool C_XtDirty::IsDirty()
	{
		return m_dirtyElements.Size() != 0;
	}

	bool C_XtDirty::IsDirty(Xt::DomElement* element)
	{
		if (m_dirtyElements.Find(element) != m_dirtyElements.End())
			return true;
		else
			return false;
	}

	int C_XtDirty::Size()
	{
		return m_dirtyElements.Size();
	}

	void C_XtDirty::SetDirty(Xt::DomElement* element)
	{
		m_dirtyElements.Insert(element);
	}

	void C_XtDirty::Clear()
	{
		m_dirtyElements.Clear();
		m_deferred = false;
	}

	bool C_XtDirty::IsDeferred()
	{
		return m_deferred;
	}

	void C_XtDirty::SetDeferred(bool deferred)
	{
		m_deferred = deferred;
	}
}