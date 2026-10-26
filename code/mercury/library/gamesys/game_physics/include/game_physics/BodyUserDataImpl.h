#ifndef GAMEPHYSICS_BODYUSERDATAIMPL_H
#define GAMEPHYSICS_BODYUSERDATAIMPL_H

#ifdef _MSC_VER
#pragma once
#endif


#include <game_physics/BodyUserData.h>


namespace GamePhysics
{

/**
@brief Helper class between C_BodyUserData and derived types.

Uses a variation on the Curiously Recurring Template Pattern, eg.
@code
class C_MyBodyUserData : public C_BodyUserDataImpl<C_MyBodyUserData>
{
	// whatever
};
@endcode
*/
template <class T_Derived, class T_Base = C_BodyUserData>
class C_BodyUserDataImpl : public T_Base
{
public:
	static U32 ClassTypeUid() {return s_TypeUid;} ///< @returns UID for this type.

protected:
	C_BodyUserDataImpl() : T_Base(s_TypeUid) {}
	~C_BodyUserDataImpl() {}

private:
	static const U32 s_TypeUid; ///< UID for this type.

	C_BodyUserDataImpl(const C_BodyUserDataImpl& copy); // can not copy
	C_BodyUserDataImpl& operator = (const C_BodyUserDataImpl& copy); // can not copy

};

template <class T_Derived, class T_Base>
const U32 C_BodyUserDataImpl<T_Derived, T_Base>::s_TypeUid = C_BodyUserData::AllocateTypeUid();

} // namespace GamePhysics


#endif // GAMEPHYSICS_BODYUSERDATAIMPL_H
