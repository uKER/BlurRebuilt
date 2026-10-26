#include "stdafx.h"

#include "Lux/2DGeometryMaths.h"

namespace Geom2D
{

Polygon::Polygon(float2 v1, float2 v2, float2 v3)
{
	nVerts = 3;
	vert[0] = v1;
	vert[1] = v2;
	vert[2] = v3;
}

#define SIDE_ON 2
#define SIDE_FRONT 0
#define SIDE_BACK 1
#define ON_EPSILON FLT_MIN

void Polygon::Clip(float fPlaneDist, float2 vPlaneNormal)
{
int		sides[MAX_2D_GEOM_POLYVERTS];
int		counts[3];
float	dot;
float	dists[MAX_2D_GEOM_POLYVERTS];
float2 p1,p2,mid;
float2 pFace[2][MAX_2D_GEOM_POLYVERTS];
int nFacePoints[2];

float2 *p = vert;

	counts[0]=counts[1]=counts[2]=0;

// determine sides for each point
	int i;
	for (i=0 ; i<nVerts ; i++)
	{
		dot = p[i].x * vPlaneNormal.x + p[i].y * vPlaneNormal.y;	// dot product
		dot -= fPlaneDist;
		dists[i] = dot;
		if (dot > ON_EPSILON)
			sides[i] = SIDE_FRONT;
		else if (dot < -ON_EPSILON)
			sides[i] = SIDE_BACK;
		else
			sides[i] = SIDE_ON;
		counts[sides[i]]++;
	}
	// wrapping
	sides[i] = sides[0];
	dists[i] = dists[0];

	// everything is on the near side
	if (counts[SIDE_FRONT] == 0)
		return;

	// everything is on the far side
	if (counts[SIDE_BACK] == 0)
	{
		nVerts = 0;
		return;
	}

	nFacePoints[0]=nFacePoints[1]=0;

	for (i=0 ; i<nVerts; i++)
	{
		p1 = p[i];

		if (sides[i] == SIDE_ON)
		{
			pFace[0][nFacePoints[0]]=p1;
			nFacePoints[0]++;
			pFace[1][nFacePoints[1]]=p1;
			nFacePoints[1]++;
			continue;
		}
	
		if (sides[i] == SIDE_FRONT)
		{
			pFace[0][nFacePoints[0]]=p1;
			nFacePoints[0]++;
		}
		else // SIDE_BACK
		{
			pFace[1][nFacePoints[1]]=p1;
			nFacePoints[1]++;
		}
		
		if (sides[i+1] == SIDE_ON || sides[i+1] == sides[i])
			continue;

	// generate a split point
		p2 = p[(i+1)%nVerts];
		
		dot = dists[i] / (dists[i]-dists[i+1]);
		mid = p1 + (p2-p1) * dot; // lerp position

		pFace[0][nFacePoints[0]]=mid;
		nFacePoints[0]++;
		pFace[1][nFacePoints[1]]=mid;
		nFacePoints[1]++;
	}

	// replace this polygon with the new near-side polygon
	int iNearFace = SIDE_BACK;
	nVerts = nFacePoints[iNearFace];
	for (int i=0; i<nVerts; i++)
	{
		vert[i] = pFace[iNearFace][i];
	}
}


}