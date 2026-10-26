#include "Stdafx.h"
#include "gravity/broadphase/DistributionTree.h"
#include "system/serialise/I_BinaryStream.h"
#include "system/serialise.h"
#include "gravity/narrowphase/CollisionManager.h"
#include "gravity/importer/importer.h"
#include "gravity/sharedphysics/PhysicsVersion.h"
#include <system/DebugLog.h>
#include "gravity/sdk/Scene.h"

#include "gravity/sdk/Body.h"
#include "gravity/sdk/shapedescriptors.h"
#include "gravity/sdk/ConstraintDescriptors.h"
#include "gravity/sdk/CompositeDescriptor.h"
#include "gravity/narrowphase/Shapes.h"

namespace Gravity
{

	C_BaseInstanceMeshNode::C_BaseInstanceMeshNode(Collision::C_TriangleMeshShape* mesh=NULL, const U32 id=0): m_Mesh(mesh), m_Id(id)
	{
	}

	C_BaseInstanceMeshNode::C_BaseInstanceMeshNode(): m_Mesh(NULL), m_Id(0)
	{
	}

	C_BaseInstanceMeshNode::~C_BaseInstanceMeshNode()
	{
		if(m_Mesh)
			delete m_Mesh;
		m_Mesh=NULL;
	}


	Collision::C_TriangleMeshShape* C_InstanceMeshResourceManager::FindBase(const U32 id)
	{
		C_BaseInstanceMeshNode* pBaseNode;
		for(S32 i=0; i<m_BaseShapes.m_numElements; ++i)
		{
			pBaseNode=m_BaseShapes[i];
			if(pBaseNode->m_Id == id)
				return pBaseNode->m_Mesh;
		}
		return NULL;
	}
	void C_InstanceMeshResourceManager::AddBase(Collision::C_TriangleMeshShape* mesh, const U32 id)
	{
		C_BaseInstanceMeshNode* node=new C_BaseInstanceMeshNode(mesh, id);
		m_BaseShapes.AddToEnd(&node);
	}

	C_InstanceMeshResourceManager::~C_InstanceMeshResourceManager()
	{
		Reset();
	}

	void C_InstanceMeshResourceManager::Reset()
	{
		for(S32 a = 0; a < m_BaseShapes.m_numElements; ++a)
		{
			if(m_BaseShapes[a])
				delete m_BaseShapes[a];
			m_BaseShapes[a] = NULL;
		}
		m_BaseShapes.m_numElements = 0;
	}
}
