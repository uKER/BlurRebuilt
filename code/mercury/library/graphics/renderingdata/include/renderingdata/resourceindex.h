#ifndef RD_RESOURCE_INDEX_H
#define RD_RESOURCE_INDEX_H
#ifdef _MSC_VER
#pragma once
#endif

#include "btl/utility/hash.h"

namespace RenderingData
{
	// Index an item in a resource block.
	class ResourceIndex
	{
	public:
		// Initialised to null.
		ResourceIndex();

		void SetBlockIndex(S32 b, S32 i);

		S32 GetBlock() const; // -1 used to indicate a shared resource.
		S32 GetIndex() const; // -1 indicates null.
		bool IsNull() const;
		bool IsShared() const;

		static ResourceIndex Null();
		static ResourceIndex BlockIndex(S32 b, S32 i);

		FRIEND_SERIALISE(ResourceIndex);

	private:
		ResourceIndex(S32 b, S32 i);

		S32 m_block;
		S32 m_index;
	};

	bool operator == (const ResourceIndex& r1, const ResourceIndex& r2);
	bool operator != (const ResourceIndex& r1, const ResourceIndex& r2);
	bool operator < (const ResourceIndex& r1, const ResourceIndex& r2);
	U32 Hash(const RenderingData::ResourceIndex& r);
}

#endif
