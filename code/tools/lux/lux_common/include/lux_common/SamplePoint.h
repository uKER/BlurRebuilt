#ifndef __SAMPLEPOINT_H__
#define __SAMPLEPOINT_H__

#define MY_MAX_SH_ORDER 3

#define MAX_SH_VECTOR_SIZE (MY_MAX_SH_ORDER * MY_MAX_SH_ORDER)

struct SamplePoint
{
	SamplePoint()
	{
		memset(this, 0, sizeof(SamplePoint));
	};

	float3	m_vPos;
	float	m_fSH_R[MAX_SH_VECTOR_SIZE];
	float	m_fSH_G[MAX_SH_VECTOR_SIZE];
	float	m_fSH_B[MAX_SH_VECTOR_SIZE];
};

extern bool gbDisplaySamplePoints;
extern Ut::Vector<SamplePoint> gSamplePoints;

#endif