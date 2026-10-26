
#ifndef CAMERA2_CAMERASCENESERIALISE_H
#define CAMERA2_CAMERASCENESERIALISE_H

#include "AnimData.h"
#include "AnimDataSerialise.h"
#include "TvZone.h"

namespace Cam2
{

// C_CameraScene:
template <typename Archive>
void Serialise(Archive& s, C_CameraScene& t)
{
	S32 i = 0, count = 0;

	count = t.m_cams.Size();
	s << count;
	for (i = 0; i < count; i++)
		s << *t.m_cams[i];

	count = t.m_zones.Size();
	s << count;
	for (i = 0; i < count; i++)
		s << *t.m_zones[i];
}

template <typename Archive>
void Deserialise(Archive& s, C_CameraScene& t)
{
	S32 i = 0, count = 0;

	s >> count;
	t.m_cams.Resize(count);

	for (i = 0; i < count; i++)
	{
		t.m_cams[i] = new C_AnimData;
		s >> *t.m_cams[i];
	}

	s >> count;
	t.m_zones.Resize(count);

	for (i = 0; i < count; i++)
	{
		t.m_zones[i] = new C_TvZone;
		s >> *t.m_zones[i];
	}
}

// C_TvZone:
template <typename Archive>
void Serialise(Archive& s, C_TvZone& t)
{
	s << t.m_point;
	s << t.m_face;
	s << t.m_cameraStart;
	s << t.m_cameraCount;
	s << t.m_spline;
}

template <typename Archive>
void Deserialise(Archive& s, C_TvZone& t)
{
	s >> t.m_point;
	s >> t.m_face;
	s >> t.m_cameraStart;
	s >> t.m_cameraCount;
	s >> t.m_spline;
}

// C_ZoneSpline:
template <typename Archive>
void Serialise(Archive& s, C_ZoneSpline& t)
{
	s << t.m_cvs;
}

template <typename Archive>
void Deserialise(Archive& s, C_ZoneSpline& t)
{
	s >> t.m_cvs;
}

// C_ZoneSpline::Cv :
template <typename Archive>
void Serialise(Archive& s, C_ZoneSpline::Cv& t)
{
	s << t.v << t.alpha;
}

template <typename Archive>
void Deserialise(Archive& s, C_ZoneSpline::Cv& t)
{
	s >> t.v >> t.alpha;
}

}
#endif
