
#ifndef CAMERA2_TVZONE_H
#define CAMERA2_TVZONE_H

#pragma warning (push, 3)
#include <btl/containers/vector.h>
#include <misc_shared/archive/archive.h>
#pragma warning (pop)

namespace Cam2
{

class C_ZoneSpline
{
public:
	// This is a spline which runs through a zone, as the player moves through the zone, it knows
	// which part of the camera animation to play
	
	class Cv
	{
	public:
		Maths::HVec3 v;
		F32 alpha; // alpha is how far through the spline this control vertex is (0.0 to 1.0)
	};
	
	Btl::Vector<Cv> m_cvs;
	F32 Progress(const Maths::HVec3& point) const;
};

class C_TvZone
{
public:
	Btl::Vector<Maths::HVec3> m_point; // Points of the 3D zone
	Btl::Vector<S32> m_face; // Points for each face
	S32 m_cameraStart, m_cameraCount; // Camera indexes for the zone
	
	// todo - spline should go with camera, not the zone:
	C_ZoneSpline m_spline; // The spline for player progress for this zone

	C_TvZone();
	~C_TvZone();
	bool InsideZone(const Maths::HVec3& point) const;
};

} // End of namespace

#endif
