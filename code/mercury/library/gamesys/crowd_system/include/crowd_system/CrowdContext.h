#ifndef CROWD_CONTEXT_H
#define CROWD_CONTEXT_H

#ifdef _MSC_VER
#pragma once
#endif

namespace AnimatedCrowd
{
	class C_CrowdResources;
};

namespace AnimatedCrowd
{

//! C_UpdateContext
class C_UpdateContext
{
public:

	C_UpdateContext()
		: m_Resources(NULL),
		m_DeltaTime(0.f),
		m_FrameCount(0),
		m_HipJointIndex(0)
	{}

	const C_CrowdResources* m_Resources;

	F32	m_DeltaTime;
	U32 m_FrameCount;
	U32	m_HipJointIndex;
};

}; // namespace AnimatedCrowd

#endif // CROWD_CONTEXT_H
