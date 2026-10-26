#ifndef __2D_GEOMETRY_MATHS_H__
#define __2D_GEOMETRY_MATHS_H__

namespace Geom2D
{

#define MAX_2D_GEOM_POLYVERTS 8

class Polygon
{
public:
	Polygon(float2 v1, float2 v2, float2 v3);
	void Clip(float x, float2 vNormal);

	float2	vert[MAX_2D_GEOM_POLYVERTS];
	int		nVerts;
};

}

#endif
