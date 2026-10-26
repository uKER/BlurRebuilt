namespace Messaging
{

/**
@param range range to validate.
@returns true if this is a valid range.
*/
inline bool Validate(const C_MessageTypeRange& range)
{
	return (range.m_Last >= range.m_First);
}

/**
@param range a valid range.
@returns the size of the range.
*/
inline U32 GetSize(const C_MessageTypeRange& range)
{
	BIZ_ASSERT(Validate(range));
	return (range.m_Last - range.m_First) + 1;
}

/**
@param range a valid range.
@param value value to check for.
@returns true if the value is within the range.
*/
inline bool Contains(const C_MessageTypeRange& range, const TMsgType value)
{
	BIZ_ASSERT(Validate(range));
	return ((value >= range.m_First) && (value <= range.m_Last));
}

/**
@param a a valid range.
@param b a valid range.
@returns true if the whole range of b is contained by a.
*/
inline bool Contains(const C_MessageTypeRange& a, const C_MessageTypeRange& b)
{
	BIZ_ASSERT(Validate(a));
	BIZ_ASSERT(Validate(b));
	return ((a.m_First <= b.m_First) && (a.m_Last >= b.m_Last));
}

/**
@param a a valid range.
@param b a valid range.
@returns true if the ranges overlap.
*/
inline bool Overlaps(const C_MessageTypeRange& a, const C_MessageTypeRange& b)
{
	BIZ_ASSERT(Validate(a));
	BIZ_ASSERT(Validate(b));
	return ((a.m_Last >= b.m_First) && (a.m_First <= b.m_Last));
}

/**
@param a a valid range.
@param b a valid range.
@returns the inclusive range between a and b.
*/
inline C_MessageTypeRange Union(const C_MessageTypeRange& a, const C_MessageTypeRange& b)
{
	BIZ_ASSERT(Validate(a));
	BIZ_ASSERT(Validate(b));

	C_MessageTypeRange result;

	result.m_First = Maths::Min(a.m_First, a.m_Last);
	result.m_Last = Maths::Max(a.m_Last, b.m_Last);

	return result;
}

/**
@param a a valid range.
@param b a valid range.
@returns the intersection of a and b, or an invalid range if a and b do not overlap.
*/
inline C_MessageTypeRange Intersection(const C_MessageTypeRange& a, const C_MessageTypeRange& b)
{
	BIZ_ASSERT(Validate(a));
	BIZ_ASSERT(Validate(b));

	C_MessageTypeRange result;

	result.m_First = Maths::Max(a.m_First, a.m_Last);
	result.m_Last = Maths::Min(a.m_Last, b.m_Last);

	return result;
}

} // namespace Messaging

