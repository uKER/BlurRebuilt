
#include "stdafx.h"
#include <camera2/tv/AnimData.h>

namespace Cam2
{

// Camera animation data class - this holds fcurve animation data exported from Maya

static int SearchForKey(const Btl::Vector<Cam2::C_AnimFrame>& frames, F32 counter)
{
	int max = frames.Size();
	int low = 0, high = max - 1, mid = 0;
	
	while (high > low + 1)
	{
		mid = (low + high) >> 1;
		if (counter < frames[mid].m_time)
			high = mid;
		else
			low = mid;
	}
	return low;
}

void C_AnimData::EvaluatePosition(F32 counter, Maths::HVec3& pos, Maths::C_Quaternion& orient) const
{
	S32 max = m_frame.Size();
	if (max <= 0)
		return;
	
	int index = SearchForKey(m_frame, counter);
	const C_AnimFrame& fr = m_frame[index];

	S32 indexb = index + 1;
	if (indexb > max - 1)
		indexb = max - 1;
	const C_AnimFrame& frb = m_frame[indexb];
	
	F32 gap = frb.m_time - fr.m_time;
	F32 lerp = counter - fr.m_time;
	if (gap > 1e-6f)
		lerp /= gap;
	
	pos = fr.m_position + lerp * (frb.m_position - fr.m_position);
	orient = fr.m_orient.ShortestLerp(frb.m_orient, lerp);
}

F32 C_AnimData::EvaluateAttribute(F32 counter, E_Attr attribute) const
{
	S32 max = m_frame.Size();
	if (max <= 0)
		return 0.f;

	int index = SearchForKey(m_frame, counter);
	const C_AnimFrame& fr = m_frame[index];

	S32 indexb = index + 1;
	if (indexb > max - 1)
		indexb = max - 1;
	const C_AnimFrame& frb = m_frame[indexb];

	F32 gap = frb.m_time - fr.m_time;
	F32 lerp = counter - fr.m_time;
	if (gap > 1e-6f)
		lerp /= gap;

	F32 value = 0.f;
	if (fr.m_attr.Size() >= attribute + 1 && frb.m_attr.Size() >= attribute + 1)
	{
		value = fr.m_attr[attribute] * (1.f - lerp) + frb.m_attr[attribute] * lerp;
	}

	return value;
}

F32 C_AnimData::CalcLength() const
{
	if (m_frame.Size() <= 0)
		return 0.f;
		
	return m_frame.Back().m_time;
}

// ----------------------------------------------------------------
C_AnimFrame::C_AnimFrame()
{
	m_time = 0.f;
	m_attr.Resize(3);
	S32 i = 0;
	for (i = 0; i < m_attr.Size(); i++)
		m_attr[i] = 0.f;
}

void C_AnimFrame::SetAttr(E_Attr type, F32 value)
{
	// Make sure m_attr vector is big enough
	if (m_attr.Size() < type + 1)
	{
		m_attr.Resize(type + 1);
		if (m_attr.Size() < type + 1)
			return;
	}
	
	// Set value:
	m_attr[type] = value;
}

} // End of namespace
