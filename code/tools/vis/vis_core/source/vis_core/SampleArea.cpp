#include "stdafx.h"

#include "vis_core/SampleArea.h"
#include "vis_core/Sampler.h"

IMPLEMENT_PERSISTENCE(SampleList, "SList");

// area containing a list of samples
bool SampleList::GetSample(int iSample, float3 &vPos, float3 &vDir)
{
	PvsPoint *p = &m_vPoints[iSample % m_vPoints.Size()];

	vPos = p->vPos;

	switch (p->iType)
	{
	case PVS_TYPE_POSITION:
		switch (iSample / m_vPoints.Size())
		{
		default:	vDir = float3( 1, 0, 0);	break;
		case 1:		vDir = float3( 0, 0, 1);	break;
		case 2:		vDir = float3(-1, 0, 0);	break;
		case 3:		vDir = float3( 0, 0,-1);	break;
		case 4:		vDir = float3( 0, 1, 0);	break;
		}
		return true;
	case PVS_TYPE_DIRECTION:
		vDir = p->vDir;
		return iSample < m_vPoints.Size();
	}
}
