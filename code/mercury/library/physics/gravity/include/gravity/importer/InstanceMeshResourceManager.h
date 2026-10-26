#ifndef INSTANCEMESHRESOURCEMANAGER_H
#define INSTANCEMESHRESOURCEMANAGER_H

#include "gravity/sharedphysics/sortedarray.h"
//#include "gravity/sharedphysics/sortedarray.inl"
//#include "gravity/narrowphase/Shapes.h"

namespace Collision
{
	class C_TriangleMeshShape;
};

namespace Gravity
{

	/*class C_InstanceMeshNode
	{
	public:
		U32 m_Id;
		U32 m_BaseId;
		Maths::HMat43 m_Transform;
	};*/

	class C_BaseInstanceMeshNode
	{
	public:
		C_BaseInstanceMeshNode(Collision::C_TriangleMeshShape* mesh, const U32 id);

		C_BaseInstanceMeshNode();

		~C_BaseInstanceMeshNode();

		Collision::C_TriangleMeshShape* m_Mesh;
		U32 m_Id;
	};

	class C_InstanceMeshResourceManager
	{
	public:
		C_InstanceMeshResourceManager()
		{
		}

		~C_InstanceMeshResourceManager();
		
		Collision::C_TriangleMeshShape* FindBase(const U32 id);

		void AddBase(Collision::C_TriangleMeshShape* mesh, const U32 id);

		/*void AddInstance(const U32 id, const U32 baseId, const Maths::HMat43 PCREF transform)
		{
			C_InstanceMeshNode* pInstance=new C_InstanceMeshNode;
			pInstance->m_Id = id;
			pInstance->m_BaseId = baseId;
			pInstance->m_Transform = transform;
			m_Instances.AddToEnd(&pInstance);
		}

		C_InstanceMeshNode* GetInstance(U32 index)
		{
			return m_Instances[index];
		}*/

		void Reset();
	private:
		SharedPhysics::SortedArray< C_BaseInstanceMeshNode*> m_BaseShapes;
		//SharedPhysics::SortedArray< C_InstanceMeshNode*> m_Instances;
	};
};
#endif

