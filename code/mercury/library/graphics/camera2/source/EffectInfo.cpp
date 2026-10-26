
#include "stdafx.h"
#include <camera2/EffectInfo.h>

namespace Cam2
{

C_EffectInfo::C_EffectInfo()
{
	m_NearStart = m_NearEnd = 0.f;
	m_FarStart = m_FarEnd = m_FarScale = 0.f;
}

void C_EffectInfo::SetInfo(F32 nearStart, F32 nearEnd, F32 farStart, F32 farEnd, F32 farScale)
{
	m_NearStart = nearStart;
	m_NearEnd = nearEnd;
	m_FarStart = farStart;
	m_FarEnd = farEnd;
	m_FarScale = farScale;
}

} // End of namespace
