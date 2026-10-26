#ifndef EDITOR_LIB_NODE_ID
#define EDITOR_LIB_NODE_ID

#ifdef _MSC_VER_
#pragma once
#endif

#include <Xt/XtCoreTypes.h>

namespace EditorLib
{
	const XtId			kInvalidXtId = 0;
	const U32			kNoChildId = 0xffffffff;

	class C_NodeId
	{
	public:
		FORCE_INLINE C_NodeId();
		FORCE_INLINE C_NodeId(const C_NodeId& rhs);
		FORCE_INLINE C_NodeId(XtId xtId, U32 childId);
		C_NodeId(const char* str);

		FORCE_INLINE const C_NodeId& operator=(const C_NodeId& rhs);

		FORCE_INLINE bool operator<(const C_NodeId& rhs) const;

		FORCE_INLINE bool operator==(const C_NodeId& rhs) const;

		FORCE_INLINE bool operator!=(const C_NodeId& rhs) const;

		void FromString(const char* str);

		void FromString(const Btl::String& str);

		const Btl::String& AsString() const;

		XtId GetXtId() const;

		U32 GetChildId() const;

		bool IsValid() const;

	private:
		mutable bool m_idStringInvalid;
		mutable Btl::String m_idString;
		XtId m_xtId;
		U32 m_childId;
	};

	class HashKeyOpNodeId
	{
	public:
		//Hashing function.
		static unsigned int GetHash(const C_NodeId& key)
		{
			return Ut::HashKeyOpString::GetHash(key.AsString().Chars());
		}

		static bool Equal(const C_NodeId& key1, const C_NodeId& key2)
		{
			if (key1 == key2)
				return true;

			return false;
		}
	};
	#include "NodeId.inl"
}

#endif
