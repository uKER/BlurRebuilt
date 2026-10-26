#ifndef DMAN_DOM_ID
#define DMAN_DOM_ID

#ifdef _MSC_VER_
#pragma once
#endif

namespace DMan
{
	class C_DomId
	{
	public:
		FORCE_INLINE C_DomId();
		FORCE_INLINE C_DomId(const C_DomId& rhs);
		FORCE_INLINE C_DomId(XtId packId, XtId recordId, S32 childId = -1);

		FORCE_INLINE const C_DomId& operator=(const C_DomId& rhs);

		FORCE_INLINE bool operator<(const C_DomId& rhs) const;

		FORCE_INLINE bool operator==(const C_DomId& rhs) const;

		FORCE_INLINE bool operator!=(const C_DomId& rhs) const;

		void FromString(const Btl::String& str);

		Btl::String AsString() const;

		XtId GetPackId() const;

		XtId GetRecordId() const;

		S32 GetChildId() const;

		bool IsValid() const;

	private:
		XtId	m_packId;
		XtId	m_recordId;
		S32		m_childId;
	};

	class HashKeyOpNodeId
	{
	public:
		//Hashing function.
		static unsigned int GetHash(const C_DomId& key)
		{
			return Btl::HashKeyOpBtlString::GetHash(key.AsString().Chars());
		}

		static bool Equal(const C_DomId& key1, const C_DomId& key2)
		{
			if (key1 == key2)
				return true;

			return false;
		}
	};
	#include "DomId.inl"
}

#endif //DMAN_DOM_ID
