#include "stdafx.h"

#include "Vis_Core\Mesh.h"
#include "Vis_Core\Weld.h"

IMPLEMENT_PERSISTENCE(MeshNode, "Mesh");

MeshNode::MeshNode()
{
}

MeshNode::~MeshNode()
{
}

// add a polygon to this meshnode
void MeshNode::AddPolygon(float3 &v1, float3 &v2, float3 &v3, FaceFlags flags)
{
	int i = m_VertexList.Size();

	m_VertexList.PushBack();
	m_VertexList.PushBack();
	m_VertexList.PushBack();

	m_VertexList[i+0].vPos = v1;
	m_VertexList[i+1].vPos = v2;
	m_VertexList[i+2].vPos = v3;

	m_bbox.UpdateBox(v1);
	m_bbox.UpdateBox(v2);
	m_bbox.UpdateBox(v3);
	
	m_VertexList[i+0].flags =
	m_VertexList[i+1].flags =
	m_VertexList[i+2].flags = flags;

	m_Indices.PushBack(i);
	m_Indices.PushBack(i+1);
	m_Indices.PushBack(i+2);
}

FaceFlags MeshNode::GetFaceFlags(U32 faceIndex) const
{
	//All of the vertices for a face have the same flags, so lets return the flags for the first
	//vertex of the desired triangle.
	return m_VertexList[faceIndex*3].flags;
}

// delete vertex buffer + indices from this meshnode to save memory
void MeshNode::DeleteBuffers()
{
	m_VertexList.Destroy();
	m_Indices.Destroy();
}

// returns a pointer to this node's bounding box
box3f *MeshNode::GetBoundingBox()
{
	return &m_bbox;
}

// perform a weld on this mesh
void MeshNode::DoWeld()
{
	Weld weld(m_VertexList);

	Ut::Vector<Vertex> &newvtxlist = weld.GetOutputVtxList();
	Ut::Vector<U16>	  &newindices = weld.GetOutputIndices();

	// copy new vertex list
	assert(m_VertexList.Size() > newvtxlist.Size());
	m_VertexList.Resize( newvtxlist.Size() );
	memcpy(&m_VertexList[0], &newvtxlist[0], newvtxlist.Size() * sizeof(Vertex));

	// copy new indices
	assert(m_Indices.Size() == newindices.Size());
	memcpy(&m_Indices[0], &newindices[0], newindices.Size() * sizeof(U16));
}

// serialization code
void MeshNode::write( ost::Engine &ar )
{
	ar << m_bbox.vmin << m_bbox.vmax << m_iAftkLayer;
	ar << m_VertexList.Size();
	ar.writeBinary( (BYTE*)&m_VertexList[0], m_VertexList.Size() * sizeof(Vertex) );
	ar << m_Indices.Size();
	ar.writeBinary( (BYTE*)&m_Indices[0], m_Indices.Size() * sizeof(U16) );
}
void MeshNode::read( ost::Engine &ar )
{
	ar >> m_bbox.vmin >> m_bbox.vmax >> m_iAftkLayer;
	int n;
	ar >> n; m_VertexList.Resize(n); m_VertexList.Resize(n);
	ar.readBinary( (BYTE*)&m_VertexList[0], m_VertexList.Size() * sizeof(Vertex) );
	ar >> n; m_Indices.Resize(n); m_Indices.Resize(n);
	ar.readBinary( (BYTE*)&m_Indices[0], m_Indices.Size() * sizeof(U16) );
}

