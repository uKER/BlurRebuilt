#ifndef __MESH_H__
#define __MESH_H__

#include <vis_core/FaceFlags.h>
#include <vis_core/Vertex.h>

class MeshNode : public BaseObject
{
public:
	DECLARE_PERSISTENCE(MeshNode);

	MeshNode();
	~MeshNode();

	void	AddPolygon(
		float3 &v1,
		float3 &v2,
		float3 &v3,
		FaceFlags flags);

	Ut::Vector<Vertex>	&GetVertices()	{ return m_VertexList; }
	Ut::Vector<U16>	&GetIndices()		{ return m_Indices; }
	void	DeleteBuffers();
	box3f	*GetBoundingBox();

	FaceFlags	GetFaceFlags(U32 faceIndex) const;

	void	DoWeld();

	void SetAftkLayer(int i) { m_iAftkLayer = i; }
	int GetAftkLayer() { return m_iAftkLayer; }

	void write( ost::Engine &ar );
	void read ( ost::Engine &ar );

protected:
    box3f	m_bbox;
	int		m_iAftkLayer;

	Ut::Vector<Vertex>	m_VertexList;
	Ut::Vector<U16>		m_Indices;
};



#endif