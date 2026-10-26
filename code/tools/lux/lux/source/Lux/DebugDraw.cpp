#include "stdafx.h"

int gnLines = 0;
#define MAX_LINES 1024*256
float3 gvLines[MAX_LINES];
void AddLine(float3 vs, float3 ve)
{
	if (gnLines>=MAX_LINES)
		return;
	gvLines[gnLines+0] = vs;
	gvLines[gnLines+1] = ve;
	gnLines+=2;
}

void AddBoundingBox(const box3f &bbox)
{
	float3 v[8];
	for (int i=0; i<8; i++)
		v[i] = float3( (i&1)?bbox.vmin.x:bbox.vmax.x, (i&2)?bbox.vmin.y:bbox.vmax.y, (i&4)?bbox.vmin.z:bbox.vmax.z);

	AddLine(v[0],v[1]);
	AddLine(v[1],v[3]);
	AddLine(v[3],v[2]);
	AddLine(v[2],v[0]);

	AddLine(v[4+0],v[4+1]);
	AddLine(v[4+1],v[4+3]);
	AddLine(v[4+3],v[4+2]);
	AddLine(v[4+2],v[4+0]);

	AddLine(v[4+0],v[0+0]);
	AddLine(v[4+1],v[0+1]);
	AddLine(v[4+2],v[0+2]);
	AddLine(v[4+3],v[0+3]);
}

void AddLine(Float3 vs, Float3 ve)
{
	AddLine((float3)vs,(float3)ve);
}

void AddBoundingBox(const Box3 &bbox)
{
	AddBoundingBox((const box3f)bbox);
}
