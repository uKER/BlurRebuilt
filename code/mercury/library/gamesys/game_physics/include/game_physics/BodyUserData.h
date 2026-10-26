#ifndef GAMEPHYSICS_BODYUSERDATA_H
#define GAMEPHYSICS_BODYUSERDATA_H

#ifdef _MSC_VER
#pragma once
#endif


#include <game_physics/CollisionFilter.h>


namespace GamePhysics
{

/**
@brief Base class for objects used a user data for gravity.

@note never delete a pointer to this base class, the dtor is not virtual.
*/
class C_BodyUserData
{
public:
	U32							GetTypeUid() const {return m_TypeUid;} ///< @returns type UID for this object.

	const C_CollisionFilter&	GetCollisionFilter() const {return m_CollisionFilter;} ///< @returns collision filter.
	void						SetCollisionFilter(const C_CollisionFilter& value) {m_CollisionFilter = value;} ///< @param value collision filter.

protected:
	explicit C_BodyUserData(const U32 typeUid);
	~C_BodyUserData();

	static U32					AllocateTypeUid();

private:
	U32							m_TypeUid; ///< UID
	C_CollisionFilter			m_CollisionFilter; ///< filter for collision events

	C_BodyUserData(const C_BodyUserData& copy); // can not copy
	C_BodyUserData& operator = (const C_BodyUserData& copy); // can not copy

};

/// @param typeUid unique type identifier.
inline C_BodyUserData::C_BodyUserData(const U32 typeUid)
	: m_TypeUid(typeUid)
	, m_CollisionFilter()
{
}

inline C_BodyUserData::~C_BodyUserData()
{
}

} // namespace GamePhysics


#endif // GAMEPHYSICS_BODYUSERDATA_H
