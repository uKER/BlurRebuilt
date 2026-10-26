#include <stdafx.h>

using namespace Aftk;

C_LimitMixin::C_LimitMixin()
{
	OneTimeInit();
}

C_LimitMixin::C_LimitMixin(const C_LimitMixin& rhs)
{
	OneTimeInit();
	*this = rhs;
}

C_LimitMixin::~C_LimitMixin()
{
}

const C_LimitMixin& C_LimitMixin::operator=(const C_LimitMixin& rhs)
{
	memcpy(m_limits, rhs.m_limits, sizeof(F32)*E_LimitType_Count);
	memcpy(m_isLimited, rhs.m_limits, sizeof(bool)*E_LimitType_Count);

	return *this;
}

void C_LimitMixin::SetIsLimited(E_LimitType type, bool isLimited)
{
	m_isLimited[type] = isLimited;
}

void C_LimitMixin::SetLimit(E_LimitType type, F32 limit)
{
	m_limits[type] = limit;
}

bool C_LimitMixin::IsLimited(E_LimitType type) const
{
	return m_isLimited[type];
}

F32 C_LimitMixin::GetLimit(E_LimitType type) const
{
	return m_limits[type];
}

void C_LimitMixin::OneTimeInit()
{
	memset(m_limits, 0, sizeof(F32)*E_LimitType_Count);
	memset(m_isLimited, 0, sizeof(bool)*E_LimitType_Count);
}

