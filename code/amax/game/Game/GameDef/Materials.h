#ifndef GAME_MATERIALS_H
#define GAME_MATERIALS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace SharedPhysics { struct C_MaterialPair; }

namespace Game
{

// E_EffectMaterial

enum E_EffectMaterial
{
	E_EffectMaterial_Default = 0,
	E_EffectMaterial_Tarmac,
	E_EffectMaterial_Concrete,
	E_EffectMaterial_Rock,
	E_EffectMaterial_Dirt,
	E_EffectMaterial_Mud,
	E_EffectMaterial_Grass,
	E_EffectMaterial_Gravel,
	E_EffectMaterial_Sand,
	E_EffectMaterial_WetSand,
	E_EffectMaterial_Water,
	E_EffectMaterial_Brick,
	E_EffectMaterial_Vegetation,
	E_EffectMaterial_Wood,
	E_EffectMaterial_Metal,
	E_EffectMaterial_Plastic,
	E_EffectMaterial_Glass,
	E_EffectMaterial_Rubber,
	E_EffectMaterial_Upholstery,
	E_EffectMaterial_Cardboard, 
	E_EffectMaterial_Flesh,
	E_EffectMaterial_TrainTracks,
	E_EffectMaterial_WaterDeep,
	E_EffectMaterial_ConcreteFenceChain,
	E_EffectMaterial_ConcreteFencePlastic,
	E_EffectMaterial_MetalHollow,
	E_EffectMaterial_MetalArmco,
	E_EffectMaterial_MetalSheet,
	E_EffectMaterial_MetalFenceChain,
	E_EffectMaterial_MetalFenceSolid,
	E_EffectMaterial_MetalFenceSlatted,
	E_EffectMaterial_MetalSolidPost,
	E_EffectMaterial_DirtLightRough,
	E_EffectMaterial_DirtHeavyRough,
	E_EffectMaterial_WackyMagic,
	E_EffectMaterial_PavingStone,	

	E_EffectMaterial_Vehicle,

	E_EffectMaterial_Count
};

const Util::C_String GetEffectMaterialName( const E_EffectMaterial material );

// cast to enum type and limit range
const E_EffectMaterial GetEffectMaterial( const S32 iMaterial );

// E_PhysicsMaterial

enum E_PhysicsMaterial
{
	E_PhysicsMaterial_Default = 0,
	E_PhysicsMaterial_MediumFriction,
	E_PhysicsMaterial_LowFriction,
	E_PhysicsMaterial_Bouncy,
	E_PhysicsMaterial_Vehicle,
	E_PhysicsMaterial_Object,	
	E_PhysicsMaterial_ObjectBouncy,

	// this is only used ingame so it can be kept at the end of the enum
	E_PhysicsMaterial_CharacterController,
	E_PhysicsMaterial_Anisotropic_Wall, //Only used against vehicles!!!
	E_PhysicsMaterial_Anisotropic_ZeroY, //Only used against vehicles and walls when falling...
	E_PhysicsMaterial_Count
};

const Util::C_String GetPhysicsMaterialName( const E_PhysicsMaterial material );

// cast to enum type and limit range
const E_PhysicsMaterial GetPhysicsMaterial( const S32 iMaterial );

class C_PhysicsMaterials
{
public:
	static bool SetupMaterials();

	static SharedPhysics::C_MaterialPair* GetMaterialMatrix();
	static const U32 GetMaterialCount();

	static SharedPhysics::C_MaterialPair& GetMaterialPair( const U32 a, const U32 b );

private:
	static U32 m_MaterialCount;
	static SharedPhysics::C_MaterialPair m_MaterialMatrix[E_PhysicsMaterial_Count][E_PhysicsMaterial_Count];
};

} // namespace Game

#endif // GAME_MATERIALS_H
