#include "MiscSharedPch.h"
#include <misc_shared/messaging/MessageTypeRange.h>


namespace Messaging
{

const C_MessageTypeRange kFullMessageTypeRange = {static_cast<TMsgType>(INT_MIN), static_cast<TMsgType>(INT_MAX)};
const C_MessageTypeRange kInvalidMessageTypeRange = {static_cast<TMsgType>(INT_MAX), static_cast<TMsgType>(INT_MIN)};




/**
@param ranges ranges to test.
@returns true if this is a valid set of ranges.
*/
bool Validate(const C_MessageTypeRanges& ranges)
{
	if (ranges.Size() > 0)
	{
		if (!Validate(ranges.Front()))
		{
			return false;
		}

		for (S32 i = 1; i < ranges.Size(); ++i)
		{
			const C_MessageTypeRange& lo = ranges[i - 1];
			const C_MessageTypeRange& hi = ranges[i];

			if (!Validate(hi))
			{
				return false;
			}

			if (Overlaps(lo, hi) || ((lo.m_Last + 1) >= hi.m_First))
			{
				return false;
			}
		}
	}

	return true;
}

/**
@param ranges ranges to test.
@returns total number of message types in all ranges.
*/
U32 CalcSize(const C_MessageTypeRanges& ranges)
{
	U32 size = 0;

	for (C_MessageTypeRanges::ConstIterator it = ranges.Begin(); it != ranges.End(); ++it)
	{
		size += GetSize(*it);
	}

	return size;
}

/**
@param ranges ranges to test.
@param value message type to look for.
@returns true if this message type is contained in any range.
*/
bool Contains(const C_MessageTypeRanges& ranges, const TMsgType value)
{
	// TODO: accelerate this with a binary search

	for (C_MessageTypeRanges::ConstIterator it = ranges.Begin(); it != ranges.End(); ++it)
	{
		if (Contains(*it, value))
		{
			return true;
		}
	}

	return false;
}

/**
@param ranges ranges to test.
@param range range of types to compare.
@returns true if this overlaps any range.
*/
bool Overlaps(const C_MessageTypeRanges& ranges, const C_MessageTypeRange& range)
{
	// TODO: accelerate this with a binary search

	for (C_MessageTypeRanges::ConstIterator it = ranges.Begin(); it != ranges.End(); ++it)
	{
		if (Overlaps(*it, range))
		{
			return true;
		}
	}

	return false;
}


/**
@param dest ranges to insert into.
@param pData array of ranges to insert, must not be NULL.
@param count number of ranges to insert, must be greater than zero.
*/
void Insert(C_MessageTypeRanges& dest, const C_MessageTypeRange* pData, const U32 count)
{
	BIZ_ASSERT(pData != NULL);
	BIZ_ASSERT(count > 0);

	// make sure we don't reallocate multiple times
	if (count > 1)
	{
		dest.Reserve(dest.Size() + count);
	}

	// merge the two sets of ranges
	S32 oldIndex = 0;
	S32 newIndex = 0;

	while ((newIndex < static_cast<S32>(count)) && (oldIndex < dest.Size()))
	{
		const C_MessageTypeRange& oldRange = dest[oldIndex];
		const C_MessageTypeRange& newRange = pData[newIndex];

		if (Contains(oldRange, newRange))
		{
			// nothing to see here, the new range is already represented by the old range
			++newIndex;
		}
		else if (Overlaps(oldRange, newRange) || ((oldRange.m_Last + 1) == newRange.m_First) || (oldRange.m_First == (newRange.m_Last + 1)))
		{
			// merge the ranges
			C_MessageTypeRange mergedRange = Union(oldRange, newRange);

			S32 endIndex = oldIndex + 1;

			while (endIndex < dest.Size())
			{
				const C_MessageTypeRange& endRange = dest[endIndex];

				if (Overlaps(endRange, mergedRange) || (endRange.m_First == (mergedRange.m_Last + 1)))
				{
					mergedRange = Union(mergedRange, endRange);
					++endIndex;
				}
			}

			dest[oldIndex] = mergedRange;
			++oldIndex;
			++newIndex;

			if (oldIndex < endIndex)
			{
				const S32 clipCount = endIndex - oldIndex;
				const S32 keepCount = dest.Size() - endIndex;

				for (S32 keep = 0; keep < keepCount; ++keep)
				{
					dest[oldIndex + keep] = dest[oldIndex + keep + clipCount];
				}

				dest.Resize(dest.Size() - clipCount);
			}
		}
		else if (oldRange.m_First > newRange.m_Last)
		{
			// insert new range before old range
			dest.InsertBefore(newRange, dest.Begin() + oldIndex);
			++oldIndex;
			++newIndex;
		}
		else
		{
			// move on to next slot
			++oldIndex;
		}
	}

	// append any remaining ranges
	if (newIndex < static_cast<S32>(count))
	{
		dest.Reserve(dest.Size() + (static_cast<S32>(count) - newIndex));

		while (newIndex < static_cast<S32>(count))
		{
			dest.PushBack(pData[newIndex]);
			++newIndex;
		}
	}

	BIZ_ASSERT(Validate(dest));
}

/**
@param dest ranges to insert into.
@param value type to insert.
*/
void Insert(C_MessageTypeRanges& dest, const TMsgType value)
{
	C_MessageTypeRange range;
	range.m_First = value;
	range.m_Last = value;
	Insert(dest, &range, 1);
}

/**
@param dest ranges to insert into.
@param range range to insert.
*/
void Insert(C_MessageTypeRanges& dest, const C_MessageTypeRange& range)
{
	BIZ_ASSERT(Validate(range));
	Insert(dest, &range, 1);
}

/**
@param dest ranges to insert into.
@param ranges ranges to insert.
*/
void Insert(C_MessageTypeRanges& dest, const C_MessageTypeRanges& ranges)
{
	BIZ_ASSERT(Validate(ranges));
	if (ranges.Size() > 0)
	{
		Insert(dest, ranges.First(), ranges.Size());
	}
}

} // namespace Messaging
