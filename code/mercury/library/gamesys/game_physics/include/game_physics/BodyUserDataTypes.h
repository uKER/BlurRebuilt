#ifndef GAMEPHYSICS_BODYUSERDATATYPES_H
#define GAMEPHYSICS_BODYUSERDATATYPES_H

#ifdef _MSC_VER
#pragma once
#endif


namespace GamePhysics
{

class C_BodyUserData;

class C_BodyUserDataTypes
{
	friend class C_BodyUserData;

public:
	static U32 NumTypes() {return s_NumTypes;}

private:
	static U32 s_NumTypes; ///< number of body user data types

	C_BodyUserDataTypes(); // can not instantiate

};

} // namespace GamePhysics

#endif // GAMEPHYSICS_BODYUSERDATATYPES_H
