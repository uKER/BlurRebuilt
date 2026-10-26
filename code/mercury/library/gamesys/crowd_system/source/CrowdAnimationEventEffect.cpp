#include "CrowdSystemPch.h"

#include <Core/Containers/String.h>
#include <crowd_system/CrowdAnimationEventEffect.h>

namespace AnimatedCrowd
{

C_CrowdAnimationEventEffect::C_CrowdAnimationEventEffect()
	: m_EventHandle(NULL),
	m_pEventName(NULL)/*,
	m_JointIndex(-1)*/
{ }

void C_CrowdAnimationEventEffect::SetEvent(XtHandle<DynamicEffect::Particle::XtInterface::Event> eventHandle)
{
	m_EventHandle = eventHandle;
}

DynamicEffect::Particle::C_Event* C_CrowdAnimationEventEffect::GetEvent() const
{
	return m_EventHandle->pEvent;
}

void C_CrowdAnimationEventEffect::SetEventName(const char* pEventName)
{
	m_pEventName = pEventName;
}

const char* C_CrowdAnimationEventEffect::GetEventName() const
{
	return m_pEventName;
}

/*void C_CrowdAnimationEventEffect::SetJointIndex(const S32 jointIndex)
{
	m_JointIndex = jointIndex;
}

const S32 C_CrowdAnimationEventEffect::GetJointIndex() const
{
	return m_JointIndex;
}*/

bool C_CrowdAnimationEventEffect::IsValid() const
{
	return (m_EventHandle->bInitialised && (m_EventHandle->pEvent != NULL));// && (m_JointIndex >= 0));
}

}; // namespace AnimatedCrowd
