#ifndef RENDER_DYNAMICEFFECT_DEBUGINFO_H
#define RENDER_DYNAMICEFFECT_DEBUGINFO_H

#ifdef _MSC_VER
#pragma once
#endif

namespace DynamicEffect
{

class C_VertexBufferInfo
{
public:

#ifndef CFG_GOLD
	C_VertexBufferInfo() : m_iUsageSuccess(0), m_iUsageFailed(0) {}
	~C_VertexBufferInfo() {};

	const C_VertexBufferInfo operator + ( const C_VertexBufferInfo& rhs) const;
	const C_VertexBufferInfo operator - ( const C_VertexBufferInfo& rhs) const;
	
	void operator += (const C_VertexBufferInfo& rhs);
	void operator -= (const C_VertexBufferInfo& rhs);

	U32		m_iUsageSuccess;
	U32		m_iUsageFailed;
#endif
};

#ifndef CFG_GOLD
FORCE_INLINE const C_VertexBufferInfo C_VertexBufferInfo::operator + (const C_VertexBufferInfo& rhs) const
{
	C_VertexBufferInfo result;
	result.m_iUsageSuccess = m_iUsageSuccess + rhs.m_iUsageSuccess;
	result.m_iUsageFailed = m_iUsageFailed + rhs.m_iUsageFailed;
	
	return result;
}

FORCE_INLINE const C_VertexBufferInfo C_VertexBufferInfo::operator - (const C_VertexBufferInfo& rhs) const
{
	C_VertexBufferInfo result;
	result.m_iUsageSuccess = m_iUsageSuccess - rhs.m_iUsageSuccess;
	result.m_iUsageFailed = m_iUsageFailed - rhs.m_iUsageFailed;
	return result;
}

FORCE_INLINE void C_VertexBufferInfo::operator += (const C_VertexBufferInfo& rhs)
{
	m_iUsageSuccess += rhs.m_iUsageSuccess;
	m_iUsageFailed += rhs.m_iUsageFailed;
};

FORCE_INLINE void C_VertexBufferInfo::operator -= (const C_VertexBufferInfo& rhs)
{
	m_iUsageSuccess -= rhs.m_iUsageSuccess;
	m_iUsageFailed -= rhs.m_iUsageFailed;
};
#endif

}

#endif // RENDER_DYNAMICEFFECT_DEBUGINFO_H
