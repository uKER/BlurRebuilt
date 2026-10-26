#ifndef EDITOR_LIB_GUID_MANAGER_H
#define EDITOR_LIB_GUID_MANAGER_H

#ifdef _MSC_VER_
#pragma once
#endif

namespace EditorLib
{
	class C_GuidManager
	{
	public:
		C_GuidManager();
		~C_GuidManager();

		///Create a new id.
		bool CreateGuid(U32& id);

		///Attempt to use given id.
		bool RegisterGuid(U32& id);

		///Free an existing id.
		void DestroyGuid(U32 id);

	private:
		typedef Ut::OpenHashSet<U32> IdSet;
		IdSet m_ids;
	};
}

#endif
