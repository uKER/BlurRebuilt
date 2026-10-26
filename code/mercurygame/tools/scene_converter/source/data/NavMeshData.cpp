#include "stdafx.h"

#include <converter_shared/ProcessGraph.h>

#include <data/DataTypes.h>
#include <data/NavMeshData.h>

enum E_FileVersion
{
	E_FileVersion_Initial = 0,
	E_FileVersion_Current = E_FileVersion_Initial
};

C_DataNavMesh::C_DataNavMesh()
{
}

C_DataNavMesh::~C_DataNavMesh()
{
	for (S32 i = 0; i < m_NavMeshVector.Size(); i++)
		delete m_NavMeshVector[i];
}

const Util::C_String C_DataNavMesh::GetName() const
{
	return Util::C_String("C_NavMeshData");
}

void C_DataNavMesh::AddNavMeshVector(const Btl::Vector<const Aftk::C_SceneNode*> navMeshVector)
{
	for (S32 i = 0; i < navMeshVector.Size(); i++)
	{
		m_NavMeshVector.PushBack(NavMesh::C_Converter::Process(navMeshVector[i]));
	}
}

bool C_DataNavMesh::Export( Sys::I_File* pFile ) const
{

	Sys::BinarySerialiserAutoEndian ar( *pFile );

	ar << S32(NavMesh::E_FileVersion_Current);

	U32 nCountBase = 0;
	U32 nCountIsland = 0;
	U32 nCountStencil = 0;

	for (S32 i = 0; i < m_NavMeshVector.Size(); i++)
	{
		NavMesh::NavMeshNode* pNode = m_NavMeshVector[i];

		Util::C_String navMeshCheck(pNode->LayerName());
		navMeshCheck = navMeshCheck(0, 9);
		if(navMeshCheck == "_NAVMESH_")
		{
			Util::C_String levelName(pNode->LayerName());
			const S32 _pos = levelName.Search('_', 9);
			if(_pos == -1)
			{
				levelName = levelName(9,levelName.Length());
				if(levelName == "BASE")
				{
					nCountBase++;
				}
				else if(levelName == "ISLAND")
				{
					nCountIsland++;
				}
			}
			else
			{
				nCountStencil++;
			}
		}
	}

	ar << nCountBase;
	ar << nCountIsland;
	ar << nCountStencil;

	if (m_NavMeshVector.Size() > 0)
	{
		// Separate layers containing each game type's NavMesh
		Sys::C_FileBlock headerBlock(pFile, "NavSub");

		for (S32 i = 0; i < m_NavMeshVector.Size(); i++)
		{
			Sys::C_FileBlock meshBlock( &headerBlock, "Mesh");
			Sys::BinarySerialiserAutoEndian ar( meshBlock );
			ar << *m_NavMeshVector[i];
		}
	}

	return true;
}
