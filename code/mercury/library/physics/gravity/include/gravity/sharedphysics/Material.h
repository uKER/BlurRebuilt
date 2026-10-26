#ifndef MATERIAL_H
#define MATERIAL_H

namespace SharedPhysics
{
	struct C_MaterialPair
	{
		F32 m_Friction[3]; //Anisotropic friction - in 3D declared around x, y, and z axes
		F32 m_RestitutionCoefficient;
	};

	//typedef char MaterialDebugString[32];
	typedef const char* (*pfMaterialNameCallback)(S32);

	class C_MaterialDebug
	{
	public:
// 		C_MaterialDebug(MaterialDebugString* physicsMaterialName = NULL, U32 numPhysicsMaterial = 0, MaterialDebugString* effectMaterialName = NULL, U32 numEffectMaterial =0): m_PhysicsMaterialName(physicsMaterialName), 
// 			m_NumPhysicsMaterial(numPhysicsMaterial), m_EffectMaterialName(effectMaterialName), m_NumEffectMaterial(numEffectMaterial)
// 		{
// 
// 		}
		C_MaterialDebug() : m_PhysicsMaterialsCallback(NULL), m_EffectsMaterialsCallback(NULL) {}

// 		void SetEffectMaterialName(MaterialDebugString* effectMaterialName,  const U32 numEffectMaterial)
// 		{
// 			m_EffectMaterialName = effectMaterialName;
// 			m_NumEffectMaterial = numEffectMaterial;
// 		}
		void SetEffectMaterialCallback(pfMaterialNameCallback callback) { m_EffectsMaterialsCallback = callback; }

// 		void SetPhysicsMaterialName(MaterialDebugString* physicsMaterialName, const U32 numPhysicsMaterial)
// 		{
// 			m_PhysicsMaterialName = physicsMaterialName;
// 			m_NumPhysicsMaterial = numPhysicsMaterial;
// 		}
		void SetPhysicsMaterialCallback(pfMaterialNameCallback callback) { m_PhysicsMaterialsCallback = callback; }

		const char* GetEffectMaterialName( const U32 materialIndex )const
		{
// 			if(materialIndex >= m_NumEffectMaterial)
// 				return NULL;
// 
// 			return m_EffectMaterialName[materialIndex];
			return (*m_EffectsMaterialsCallback)(materialIndex);
		}

		const char* GetPhysicsMaterialName( const U32 materialIndex)const
		{
// 			if(materialIndex >= m_NumPhysicsMaterial)
// 				return NULL;
// 
// 			return m_PhysicsMaterialName[materialIndex];
			return (*m_PhysicsMaterialsCallback)(materialIndex);
		}

		pfMaterialNameCallback m_PhysicsMaterialsCallback;
		pfMaterialNameCallback m_EffectsMaterialsCallback;
// 		MaterialDebugString* m_PhysicsMaterialName;
// 		MaterialDebugString* m_EffectMaterialName;
// 		U32 m_NumPhysicsMaterial;
// 		U32 m_NumEffectMaterial;
	};
};

#endif

