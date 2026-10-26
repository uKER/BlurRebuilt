#ifndef XT_SHARED_XT_DIRTY_H
#define XT_SHARED_XT_DIRTY_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{
	class C_XtDirty
	{
	public:
		C_XtDirty();

		bool IsDirty();
		bool IsDirty(Xt::DomElement* element);
		int Size();
		void Clear();
		void SetDirty(Xt::DomElement* element);
		
		bool IsDeferred();
		void SetDeferred(bool deferred);

	private:
		Btl::OpenHashSet<Xt::DomElement*, Btl::HashKeyOpPtr> m_dirtyElements;
		bool m_deferred;
	};
}

#endif