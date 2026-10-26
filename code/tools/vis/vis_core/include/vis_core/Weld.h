#ifndef __WELD_H__
#define __WELD_H__

#include <map>

class VtxPos
{
public:
	VtxPos(float3 &v) : vPos(v) {};

	float3	vPos;

	bool operator<(const VtxPos &v) const
	{
		if (vPos.x<v.vPos.x)
			return true;
		else if (vPos.x>v.vPos.x)
			return false;

		if (vPos.y<v.vPos.y)
			return true;
		else if (vPos.y>v.vPos.y)
			return false;

		if (vPos.z<v.vPos.z)
			return true;
		else if (vPos.z>v.vPos.z)
			return false;

		return false;
	}
};

class Weld
{
public:
	Weld( Ut::Vector<Vertex> &vtxlist );

	Ut::Vector<Vertex>	&GetOutputVtxList();
	Ut::Vector<U16>		&GetOutputIndices();

private:

	Ut::Vector<Vertex>	&m_invtxlist;
	Ut::Vector<Vertex>	m_outvtxlist;
	Ut::Vector<U16>		m_indices;

	int		AddVertex(float3 &vPos, FaceFlags flags = FaceFlags());
	void	AddTriangle(float3 &v1, float3 &v2, float3 &v3, FaceFlags flags);

	std::map<VtxPos,int>	m_vtxmap;
};

#endif
