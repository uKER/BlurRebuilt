#include <stdafx.h>
#include <Portal/Blocker.h>
#include <Portal/DeferredBlocker.h>

using namespace Portal;


// -----------------------------------------------------------------

#ifndef PLATFORM_SPU
// Debug render the blocker.
void C_Blocker::DebugRender(DbUtil::I_DebugRenderPtr pDebugRender) const
{
	BIZ_WARNING_MSG(pDebugRender, "NULL pointer to debug renderer.");
	const Maths::RGBA green(0.0f, 1.0f, 0.0f);
	const Maths::RGBA red(1.0f, 0.0f, 0.0f);

	if(m_bActive)
	{
		pDebugRender->Draw3dCross(m_WorldMatrix.AW(), 1.0f, green);
		pDebugRender->Draw3dBox(-m_HalfExtents, m_HalfExtents, m_WorldMatrix, green);
	}
	else
	{
		pDebugRender->Draw3dCross(m_WorldMatrix.AW(), 1.0f, red);
		pDebugRender->Draw3dBox(-m_HalfExtents, m_HalfExtents, m_WorldMatrix, red);
	}
	
	return;
}
#endif // #ifndef PLATFORM_SPU

// -----------------------------------------------------------------

// Turn the blocker off.
NOALIAS void BlockerHandle::Off() __restrict
{
	BIZ_WARNING_MSG(m_pPointer, "The handle you have is invalid, probably been removed from the deferred blocker.");
	if(m_pPointer)
		m_pPointer->TurnOff();

	m_pDeferredBlocker->MarkDirty(m_pPointer);
	return;
}

// -----------------------------------------------------------------

// Turn the blocker on.
NOALIAS void BlockerHandle::On() __restrict
{
	BIZ_WARNING_MSG(m_pPointer, "The handle you have is invalid, probably been removed from the deferred blocker.");
	if(m_pPointer)
		m_pPointer->TurnOn();

	m_pDeferredBlocker->MarkDirty(m_pPointer);
	return;
}

