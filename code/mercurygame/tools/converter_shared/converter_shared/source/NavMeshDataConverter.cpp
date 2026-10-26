#include "stdafx.h"

#include "Aftk/Aftk.h"
#include "converter_shared/NavMesh/NavMesh.h"
#include "converter_shared/NavMeshDataConverter.h"

namespace NavMesh
{
	
	NavMeshNode* C_Converter::Process( const Aftk::C_SceneNode * pAftkSceneMeshNode)
	{
		return new NavMesh::NavMeshNode((Aftk::C_SceneMeshNode *)const_cast<Aftk::C_SceneNode *>(pAftkSceneMeshNode));
	}
}
