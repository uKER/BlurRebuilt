
#include "stdafx.h"
#include <camera2/tv/TvZone.h>

namespace Cam2
{

C_TvZone::C_TvZone()
{
	m_cameraStart = m_cameraCount = 0;
}

C_TvZone::~C_TvZone()
{
}

bool C_TvZone::InsideZone(const Maths::HVec3& point) const
{
	S32 fsize = m_face.Size();
	
	if (fsize < 4 * 4)
		return false; // This zone has less than 4 faces, so can't be a polyhedra
	
	S32 i = 0;
	for (i = 0; i + 4 <= fsize; i += 4)
	{
		// Get points for this face:
		Maths::HVec3 a = m_point[m_face[i    ]];
		Maths::HVec3 b = m_point[m_face[i + 1]];
		Maths::HVec3 c = m_point[m_face[i + 2]];
		
		// See if point is outside the plain of this face:
		Maths::HVec3 normal = (b - a) ^ (c - a);
		normal.Normalise();

		F32 dot = (point - a) | normal;
		if (dot > 0.f) return 0; // Outside of this face
	}
	
	return true; // Inside zone
}

// Work out the progress of an object around a zone spline
F32 C_ZoneSpline::Progress(const Maths::HVec3& point) const
{
	S32 i=0, max=0, best=0;
	F32 bestDist = FLT_MAX, bestLambda=0.f;
	F32 averageProgress = 0.f, totalWeight = 0.f;
	
	max=m_cvs.Size();
	if (max<=1) return -100.f;
	
	for (i=0; i<max-1; i++)
	{
		Maths::HVec3 a,b,x,d;
		F32 lambda=0.f,progress=0.f,weight=0.f;

		a=m_cvs[i  ].v;
		b=m_cvs[i+1].v-a;
		x=point-a;
		
		lambda=LengthSquared(b);
		if (lambda>0.001f) lambda=(b|x)/lambda;

		// Work out what the progress would be from this section of the spline:
		progress =m_cvs[i  ].alpha * (1.f-lambda);
		progress+=m_cvs[i+1].alpha * lambda;
		
		if (lambda<0.f) lambda=0.f;
		else if (lambda>1.f) lambda=1.f;
		
		d=x-b*lambda;
		
		weight=LengthSquared(d);
		if (weight<0.1f) weight=0.1f;
		weight=1.f/weight;
		
		if (LengthSquared(d)<bestDist)
		{
			best=i;
			bestDist=LengthSquared(d);
			bestLambda=lambda;
		}

		// Add to average progress value:		
		averageProgress+=progress * weight;
		totalWeight+=weight;
	}

	if (best>max-2) { i=max-2; bestLambda=1.f; } // Clip to last point

	if (totalWeight>0.0001f) averageProgress/=totalWeight;

	// Clip progress value to 0 -> 1
	if (averageProgress<0.f) averageProgress=0.f;
	else if (averageProgress>1.f) averageProgress=1.f;
	
	return averageProgress;
}


} // End of namespace
