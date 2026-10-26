#include "stdafx.h"

#include <GameLogic/GameData/GameData.h>
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

Debug::Float g_debugPhysicsMaterialDefaultFriction("Amax.PhysicsMaterials.DefaultFriction", 0.65f, Debug::Range(0.0f, 5.0f), Debug::Step(0.001f));
Debug::Float g_debugPhysicsMaterialDefaultRestitution("Amax.PhysicsMaterials.DefaultRestitution", 0.05f, Debug::Range(-1.0f, 1.0f), Debug::Step(0.001f));
Debug::Float g_debugPhysicsMaterialAnisotropicFriction("Amax.PhysicsMaterials.AnisotropicFriction", 0.05f, Debug::Range(0.f, 1.f));
Debug::Float g_debugPhysicsMaterialAnisotropicRestitution("Amax.PhysicsMaterials.AnisotropicRestitution", 0.05f, Debug::Range(0.f, 1.f));

bool C_PhysicsMaterials::SetupMaterials()
{
	m_MaterialCount = E_PhysicsMaterial_Count;

	S32 numXtMaterials = C_GameData::Instance().mFrictionSettings.Number();

	for(U32 a = 0; a < m_MaterialCount; ++a)
	{
		for(U32 b = 0; b < m_MaterialCount; ++b)
		{
			SharedPhysics::C_MaterialPair& matPair = m_MaterialMatrix[a][b];

			matPair.m_Friction[0] = matPair.m_Friction[1] = matPair.m_Friction[2] = g_debugPhysicsMaterialDefaultFriction;
			matPair.m_RestitutionCoefficient = g_debugPhysicsMaterialDefaultRestitution;

			bool found = false;
			for (S32 iMat = 0; iMat < numXtMaterials && !found; iMat++)
			{
				AmaxFrictionSettings* pSettings = C_GameData::Instance().mFrictionSettings[iMat];
				if (pSettings->material1 == a && pSettings->material2 == b || pSettings->material1 == b && pSettings->material2 == a)
				{
					matPair.m_Friction[0] = matPair.m_Friction[1] = matPair.m_Friction[2] = pSettings->friction;
					matPair.m_RestitutionCoefficient = pSettings->restitution_coefficient;
					found = true;
				}
			}			
		}
	}

	{
		SharedPhysics::C_MaterialPair& matPair = m_MaterialMatrix[E_PhysicsMaterial_Vehicle][E_PhysicsMaterial_Anisotropic_Wall];
		matPair.m_RestitutionCoefficient = (F32)g_debugPhysicsMaterialAnisotropicRestitution;
		matPair.m_Friction[0] = matPair.m_Friction[2] = (F32)g_debugPhysicsMaterialAnisotropicFriction;
		matPair.m_Friction[1] = 1.0f;
	}

	{
		SharedPhysics::C_MaterialPair& matPair = m_MaterialMatrix[E_PhysicsMaterial_Anisotropic_Wall][E_PhysicsMaterial_Vehicle];
		matPair.m_RestitutionCoefficient = (F32)g_debugPhysicsMaterialAnisotropicRestitution;
		matPair.m_Friction[0] = matPair.m_Friction[2] = (F32)g_debugPhysicsMaterialAnisotropicFriction;
		matPair.m_Friction[1] = 1.0f;
	}

	{
		SharedPhysics::C_MaterialPair& matPair = m_MaterialMatrix[E_PhysicsMaterial_Vehicle][E_PhysicsMaterial_Anisotropic_ZeroY];
		matPair.m_RestitutionCoefficient = (F32)g_debugPhysicsMaterialAnisotropicRestitution;
		matPair.m_Friction[0] = matPair.m_Friction[2] = (F32)g_debugPhysicsMaterialAnisotropicFriction;
		matPair.m_Friction[1] = 0.f;
	}

	{
		SharedPhysics::C_MaterialPair& matPair = m_MaterialMatrix[E_PhysicsMaterial_Anisotropic_ZeroY][E_PhysicsMaterial_Vehicle];
		matPair.m_RestitutionCoefficient = (F32)g_debugPhysicsMaterialAnisotropicRestitution;
		matPair.m_Friction[0] = matPair.m_Friction[2] = (F32)g_debugPhysicsMaterialAnisotropicFriction;
		matPair.m_Friction[1] = 0.f;
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
		"TrainTracks",
		"WaterDeep",
		"ConcreteFenceChain",
		"ConcreteFencePlastic",
		"MetalHollow",
		"MetalArmco",
		"MetalSheet",
		"MetalFenceChain",
		"MetalFenceSolid",
		"MetalFenceSlatted",
		"MetalSolidPost",
		"DirtLightRough",
		"DirtHeavyRough"
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
		"Vehicle",
		"Object",
		"ObjectBouncy",
		"CharacterController",
	};
 
	return materialName[material];
}

} // namespace Game

