#include "StdAfx.h"
#include "gravity/sdk/PhysicsSDK.h"
#include "gravity/importer/importer.h"

namespace Gravity
{

	C_PhysicsSDK::C_PhysicsSDK() : m_ShareVertexData(false)
	{
		
	}

	C_PhysicsSDK::~C_PhysicsSDK()
	{
	}

	C_Scene* C_PhysicsSDK::CreateScene(SharedPhysics::C_MaterialPair * materialMatrix, const U32 numMaterials)
	{
		//return m_Scenes.GetNextFreeItem();
		C_Scene* pScene = new C_Scene(materialMatrix, numMaterials);
		m_Scenes.Insert(&pScene);
		return pScene;
	}

	void C_PhysicsSDK::ReleaseScene(C_Scene* pScene)
	{
		m_Scenes.Remove(&pScene);
		delete pScene;
	}

	C_ImportScene* C_PhysicsSDK::CreateImportScene()
	{
		return new Gravity::C_ImportScene;
	}

	void C_PhysicsSDK::ReleaseImportScene(C_ImportScene* pImportScene)
	{
		if( m_Scenes.m_numElements > 0)
		{
			Gravity::C_Scene* pScene = m_Scenes[0];
			pScene->ReleaseImportScene(pImportScene);
		}
		else
		{
			delete pImportScene;
		}
	}


	U32 C_PhysicsSDK::GetNumScenes()
	{
		return m_Scenes.m_numElements;
	}

	Gravity::C_Scene* C_PhysicsSDK::GetScene(const U32 index)
	{
		return m_Scenes[index];
	}

	C_PhysicsSDK* C_PhysicsSDK::PhysicsSDK()
	{
		static C_PhysicsSDK sdk;
		return &sdk;
	}
};
