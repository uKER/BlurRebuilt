#ifndef PHYSICS_SDK_H
#define PHYSICS_SDK_H

#include "gravity/sdk/Scene.h"
#include "gravity/sdk/CollisionCallback.h"
#include "gravity/sharedphysics/SortedArray.h"

namespace Gravity
{
	//TODO - this class is going to encompass all the per-thread data (e.g. thread handles, temporary working 
	//data, all the scenes etc.). This is a singleton class, so there will only ever be one, so we can get at 
	//all this data from anywhere in the code by calling C_PhysicsSDK::PhysicsSDK()

	class C_ImportScene;
	class C_PhysicsSDK
	{
		//SharedPhysics::C_AllocationManager<C_Scene, 2, true> m_Scenes;
	private:
		C_PhysicsSDK();
		~C_PhysicsSDK();
		SharedPhysics::SortedArray<C_Scene*> m_Scenes;
		bool m_ShareVertexData;
	public:
		static C_PhysicsSDK* PhysicsSDK();

		C_Scene* CreateScene(SharedPhysics::C_MaterialPair * materialMatrix, const U32 numMaterials);
		void ReleaseScene(C_Scene* pScene);

		C_ImportScene* CreateImportScene();
		void ReleaseImportScene(C_ImportScene* pImportScene);

		U32 GetNumScenes();
		
		Gravity::C_Scene* GetScene(const U32 index);
		


		void SetShareVertexData(bool bShareVertexData)
		{
			m_ShareVertexData = bShareVertexData;
		}
		bool GetShareVertexData()
		{
			return m_ShareVertexData;
		}

	};
};

#endif
