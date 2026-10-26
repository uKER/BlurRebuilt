#include <stdafx.h>
#include <Portal/Ray.h>
#include <Portal/BSPContainer.h>
#include <Portal/DeferredTracer.h>
#include <system/Perf/PerfEvent.h>

using namespace Portal;

#ifndef PLATFORM_SPU
// Debug render the ray.
void C_Ray::DebugRender(DbUtil::I_DebugRenderPtr pDebugRender, const Maths::RGBA& rkColour) const
{
	// TODO: [Stove] implement this.
	return;
}
#endif

// -----------------------------------------------------------------

// Trace a ray immediately.
C_TraceResults C_Ray::Trace(C_BSPContainer* pContainer) const
{
#ifndef PLATFORM_PC
	// For now :)
	PERF_EVENT("portal.raytrace");
#endif
	BIZ_WARNING(pContainer);
	return pContainer->Trace(*this);
}

// -----------------------------------------------------------------

// Deferred trace of a ray -- done in a batch at next portal system update.
void C_Ray::TraceDeferred(C_DeferredTracer* pTracer, I_DeferredTracerFunctorBase* pFunctor) const
{
	BIZ_WARNING(pTracer);
	BIZ_WARNING(pFunctor);
	pTracer->AddRay(*this, pFunctor);
	return;
}
