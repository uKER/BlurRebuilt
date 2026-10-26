
#ifndef CAMERA2_ANIMDATASERIALISE_H
#define CAMERA2_ANIMDATASERIALISE_H

#pragma warning (push, 3)
#include <system/serialise.h>
#pragma warning (pop)

namespace Cam2
{

// C_AnimFrame serialise and deserialise:
template <typename Archive>
void Serialise(Archive& s, C_AnimFrame& t)
{
	s << t.m_time << t.m_position << (Maths::HVec4)t.m_orient << t.m_attr;
}

template <typename Archive>
void Deserialise(Archive &s, C_AnimFrame& t)
{
	Maths::HVec4 tmp;
	s >> t.m_time >> t.m_position >> tmp;
	t.m_orient = Maths::C_Quaternion(tmp);

	s >> t.m_attr;
}

// C_AnimData serialise and deserialise:
template <typename Archive>
void Serialise(Archive& s, C_AnimData& t)
{
	s << t.m_name;
	s << t.m_frame;
}

template <typename Archive>
void Deserialise(Archive &s, C_AnimData& t)
{
	s >> t.m_name;
	s >> t.m_frame;
}

}
#endif
