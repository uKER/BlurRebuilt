#include <stdafx.h>

#include <vis_core/FaceFlags.h>
#include <vis_core/Vertex.h>
#include <vis_core/weld.h>

// share vertices in a mesh
Weld::Weld(Ut::Vector<Vertex> &vtxlist) :
					m_invtxlist(vtxlist)
{
	// each trio of vertices represents a triangle to be added to our list
	for (int i=0; i<vtxlist.Size(); i+=3)
	{
		AddTriangle(	vtxlist[i+0].vPos,
						vtxlist[i+1].vPos,
						vtxlist[i+2].vPos, 
						vtxlist[i+0].flags);
	}
}

// retrieve output vertices
Ut::Vector<Vertex> &Weld::GetOutputVtxList()
{
	return m_outvtxlist;
}

// retrieve output indices
Ut::Vector<U16> &Weld::GetOutputIndices()
{
	return m_indices;
}

// add a triangle
void Weld::AddTriangle(float3 &v1, float3 &v2, float3 &v3, FaceFlags flags)
{
	int i1 = AddVertex(v1);
	int i2 = AddVertex(v2);
	int i3 = AddVertex(v3);

	// put iCol in one of the vertices
	if (m_outvtxlist[i1].flags.id == UNINITIALISED_FACE_ID)
	{
		m_outvtxlist[i1].flags = flags;
		m_indices.PushBack(i1);
		m_indices.PushBack(i2);
		m_indices.PushBack(i3);
		return;
	}
	if (m_outvtxlist[i2].flags.id == UNINITIALISED_FACE_ID)
	{
		m_outvtxlist[i2].flags = flags;
		m_indices.PushBack(i2);
		m_indices.PushBack(i3);
		m_indices.PushBack(i1);
		return;
	}
	if (m_outvtxlist[i3].flags.id == UNINITIALISED_FACE_ID)
	{
		m_outvtxlist[i3].flags = flags;
		m_indices.PushBack(i3);
		m_indices.PushBack(i1);
		m_indices.PushBack(i2);
		return;
	}
	// here when we need to split a vertex
	i1 = AddVertex(v1, flags);
	m_indices.PushBack(i1);
	m_indices.PushBack(i2);
	m_indices.PushBack(i3);
}

// add a vertex position, optionally force a colour
int	Weld::AddVertex(float3 &vPos, FaceFlags flags)
{
	std::map<VtxPos,int>::iterator iter;

	// see if this exact vertex position already exists
	iter = m_vtxmap.find(VtxPos(vPos));
	if (iter != m_vtxmap.end())
	{
		int idx = (*iter).second;

		if (flags.id == UNINITIALISED_FACE_ID)
			return idx;

		// set the colour
		if (m_outvtxlist[idx].flags.id == UNINITIALISED_FACE_ID)
		{
			m_outvtxlist[idx].flags = flags;
			return idx;
		}

		// if the colour was already set, split the vertex
	}

	// does not exist yet - add it
	Vertex vtx;
	vtx.vPos = vPos;
	vtx.flags = flags;
	int idx = m_outvtxlist.Size();
	m_vtxmap[VtxPos(vPos)] = idx;
	m_outvtxlist.PushBack(vtx);
	return idx;
}
