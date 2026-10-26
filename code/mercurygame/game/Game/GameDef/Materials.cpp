#include "stdafx.h"

#include <Game/GameDef/Polyflags.h>
#include <Game/GameDef/Materials.h>
#include <gravity/sdk/PhysicsSDK.h>

namespace Game
{

const E_EffectMaterial GetEffectMaterial( const S32 iMaterial )
{
	if((iMaterial < 0) || (iMaterial>=E_EffectMaterial_Count))
		return E_EffectMaterial_Default;

	return (E_EffectMaterial)iMaterial;
}

const E_PhysicsMaterial GetPhysicsMaterial( const S32 iMaterial )
{
	if((iMaterial < 0) || (iMaterial>=E_PhysicsMaterial_Count))
		return E_PhysicsMaterial_Default;

	return (E_PhysicsMaterial)iMaterial;
}

U32 C_PhysicsMaterials::m_MaterialCount = 0;
SharedPhysics::C_MaterialPair C_PhysicsMaterials::m_MaterialMatrix[E_PhysicsMaterial_Count][E_PhysicsMaterial_Count];

bool C_PhysicsMaterials::SetupMaterials()
{
	m_MaterialCount = E_PhysicsMaterial_Count;

	for(U32 a = 0; a < m_MaterialCount; ++a)
	{
		for(U32 b = 0; b < m_MaterialCount; ++b)
		{
			SharedPhysics::C_MaterialPair& matPair = m_MaterialMatrix[a][b];
			matPair.m_Friction = 0.65f;//(g_StaticFriction[a][b] + g_DynamicFriction[a][b]) * 0.8f;
			matPair.m_RestitutionCoefficient = 0.1f;//g_Restitution[a][b];
		}
	}

	for(U32 a = 0; a < m_MaterialCount; ++a)
	{
		SharedPhysics::C_MaterialPair& matPair = m_MaterialMatrix[E_PhysicsMaterial_CharacterController][a];
		matPair.m_Friction = 0.f;
		matPair.m_RestitutionCoefficient = 0.f;

		SharedPhysics::C_MaterialPair& matPair2 = m_MaterialMatrix[a][E_PhysicsMaterial_CharacterController];
		matPair2.m_Friction = 0.f;
		matPair2.m_RestitutionCoefficient = 0.f;
	}

	return true;
}

SharedPhysics::C_MaterialPair* C_PhysicsMaterials::GetMaterialMatrix()
{
	return &m_MaterialMatrix[0][0];
}

const U32 C_PhysicsMaterials::GetMaterialCount()
{
	return m_MaterialCount;
}

const Util::C_String GetEffectMaterialName( const E_EffectMaterial material )
{
	if((U32)material >= E_EffectMaterial_Count)
		return "Invalid";

	const char materialName[E_EffectMaterial_Count][32] =
	{
		"Default",
		"Tarmac",
		"Concrete",
		"Rock",
		"Dirt",
		"Mud",
		"Grass",
		"Gravel",
		"Sand",
		"WetSand",
		"Water",
		"Brick",
		"Vegetation",
		"Wood",
		"Metal",
		"Plastic",
		"Glass",
		"Rubber",
		"Upholstery",
		"Cardboard",
		"Flesh",
	};
 
	return materialName[material];
}

const Util::C_String GetPhysicsMaterialName( const E_PhysicsMaterial material )
{
	if((U32)material >= E_EffectMaterial_Count)
		return "Invalid";

	const char materialName[E_EffectMaterial_Count][32] =
	{
		"Default",		
		"MediumFriction",
		"LowFriction",
		"Bouncy",
		"CharacterController",
	};
	return materialName[material];
}

} // namespace Game
