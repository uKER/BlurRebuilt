#ifndef APP_NAVMESH_DATA_H
#define APP_NAVMESH_DATA_H

#ifdef _MSC_VER
#pragma once
#endif

#include <converter_shared/config/PolyflagConfig.h>
#include <converter_shared/NavMesh/NavMesh.h>
#include <converter_shared/NavMeshDataConverter.h>


class C_DataNavMesh : public Tools::ProcessGraph::C_Data
{
public:
	C_DataNavMesh();
	~C_DataNavMesh();

	// inherited from C_Data
	const Util::C_String GetName() const;

	void AddNavMeshVector(const Btl::Vector<const Aftk::C_SceneNode*> navMeshVector);

	bool Export( Sys::I_File* pFile ) const;

private:
	Btl::Vector<NavMesh::NavMeshNode*> m_NavMeshVector;
};

#endif

