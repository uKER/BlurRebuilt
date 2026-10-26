#ifndef EDITOR_LIB_XT_DIRTY_H
#define EDITOR_LIB_XT_DIRTY_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{
	class C_XtDirty
	{
	public:
		C_XtDirty();

		bool IsDirty() const;
		bool IsDirty(const Xt::DomElement* pElement) const;
		void Clear();
		void SetDirty(const Xt::DomElement* pElement);
		void SetDirty();
		
	private:
		Btl::OpenHashSet<const Xt::DomElement*, Btl::HashKeyOpPtr> m_dirtyElements;
		bool m_allDirty;
	};
}

#endif