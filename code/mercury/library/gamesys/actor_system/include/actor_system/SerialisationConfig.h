#ifndef SERIALISATIONCONFIG_D9FEC831_AB8B_4926_AC18_81E2899C1806_H
#define SERIALISATIONCONFIG_D9FEC831_AB8B_4926_AC18_81E2899C1806_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorDeclare.h>


namespace ActorSys
{

/**
@brief Configuration for serialisation.
@ingroup actor_group_serialisation
*/
//class C_SerialisationConfig
//{
//public:
//	enum E_Flag
//	{
//		E_Flag_IncludeCosmetic		= 1 << 0,	///< include cosmetic details that are allowed to diverge in networked games
//	};
//
//	C_SerialisationConfig() : m_Flags(0) {}
//
//	inline bool TestFlag(const E_Flag flag) const {return (m_Flags & flag) != 0;}
//	inline void SetFlag(const E_Flag flag) {m_Flags |= flag;}
//	inline void ClearFlag(const E_Flag flag) {m_Flags &= ~flag;}
//
//	inline bool IncludeCosmetic() const {return TestFlag(E_Flag_IncludeCosmetic);}
//
//private:
//	U32	m_Flags; ///< storage for flag bits
//
//};

} // namespace ActorSys

#endif // SERIALISATIONCONFIG_D9FEC831_AB8B_4926_AC18_81E2899C1806_H
