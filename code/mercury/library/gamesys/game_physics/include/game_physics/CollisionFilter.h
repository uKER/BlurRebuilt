#ifndef GAMEPHYSICS_COLLISIONFILTER_H
#define GAMEPHYSICS_COLLISIONFILTER_H

#ifdef _MSC_VER
#pragma once
#endif


namespace GamePhysics
{

/**
@brief Provides course filtering of collision events.

@note Contact types are provided by the SharedPhysics::C_ContactType 
enumeration, which at the time of writing was inside a big header that I 
didn't want to pull in so I'm just using integers for now.
*/
class C_CollisionFilter
{
public:
	explicit C_CollisionFilter(const U32 contactTypeBitfield = 0xffffffff, const F32 minImpulseMagnitude = 0.0f);
	~C_CollisionFilter();

	C_CollisionFilter(const C_CollisionFilter& copy);
	C_CollisionFilter& operator = (const C_CollisionFilter& copy);

	bool		Match(const U32 contactType, const F32 impulseMagnitude) const; ///< Test if we're interested in this collision.
	bool		MatchFluid() const {return (m_Bitfield & kFluidFlag) != 0;} ///< Test if we're interested in fluid events.

	void		SetContactTypeBitfield(const U32 value) {m_Bitfield = (value & kContactMask) | (m_Bitfield & kFlagMask);} ///< @param value bitfield of contact types we're interested in.
	void		AddContactType(const U32 contactType) {m_Bitfield |= MakeContactTypeMask(contactType);} ///< @param contactType contact type we're interested in.
	void		RemoveContactType(const U32 contactType) {m_Bitfield &= ~MakeContactTypeMask(contactType);} ///< @param contactType contact type we're not interested in.

	void		SetMinImpulseMagnitude(const F32 value) {m_MinImpulseMagnitude = value;} ///< @param value minimum magnitude of impulse we're interested in

	void		AcceptFluidEvents() {m_Bitfield |= kFluidFlag;} ///< We're interested in fluid events.
	void		RejectFluidEvents() {m_Bitfield &= ~kFluidFlag;} ///< We're not interested in fluid events.

	static U32	MakeContactTypeMask(const U32 contactType); ///@ Make a bit mask for this contact type.

private:
	enum
	{
		kFluidFlag		= 0x80000000,
		kFlagMask		= kFluidFlag,

		kContactBits	= 31, 
		kContactMask	= (1U << kContactBits) - 1,
	};

	U32			m_Bitfield; ///< bitfield indexed by contact types we're interested in and by special flags
	F32			m_MinImpulseMagnitude; ///< minimum magnitude of impulse we're interested in

};

/**
@param contactTypeBitfield bitfield of contact types we're interested in.
@param minImpulseMagnitude minimum magnitude of impulse we're interested in
*/
inline C_CollisionFilter::C_CollisionFilter(const U32 contactTypeBitfield, const F32 minImpulseMagnitude)
	: m_Bitfield((contactTypeBitfield & kContactMask) | kFluidFlag)
	, m_MinImpulseMagnitude(minImpulseMagnitude)
{
	StaticAssert((kFlagMask & kContactMask) == 0, FLAGS_AND_CONTACTS_MUST_NOT_OVERLAP_IN_BITFIELD);
	StaticAssert((kFlagMask | kContactMask) == 0xffffffff, FLAGS_AND_CONTACTS_SHOULD_USE_WHOLE_BITFIELD);
}

inline C_CollisionFilter::~C_CollisionFilter()
{
}

/// @param copy object to copy.
inline C_CollisionFilter::C_CollisionFilter(const C_CollisionFilter& copy)
	: m_Bitfield(copy.m_Bitfield)
	, m_MinImpulseMagnitude(copy.m_MinImpulseMagnitude)
{
}

/**
@param copy object to copy.
@returns this object after assignment.
*/
inline C_CollisionFilter& C_CollisionFilter::operator = (const C_CollisionFilter& copy)
{
	m_Bitfield = copy.m_Bitfield;
	m_MinImpulseMagnitude = copy.m_MinImpulseMagnitude;
	return *this;
}

/**
@param contactType contact type.
@returns bitmask for this contact type.
*/
inline U32 C_CollisionFilter::MakeContactTypeMask(const U32 contactType)
{
	BIZ_ASSERT(contactType < kContactBits);
	return (1U << contactType);
}

/**
@param contactType contact type of collision.
@param impulseMagnitude magnitude of impulse from collision.
@returns true if we're interested in this collision.
*/
inline bool C_CollisionFilter::Match(const U32 contactType, const F32 impulseMagnitude) const
{
	return (((m_Bitfield & MakeContactTypeMask(contactType)) != 0) &&
			(impulseMagnitude >= m_MinImpulseMagnitude));
}

} // namespace GamePhysics


#endif // GAMEPHYSICS_COLLISIONFILTER_H

