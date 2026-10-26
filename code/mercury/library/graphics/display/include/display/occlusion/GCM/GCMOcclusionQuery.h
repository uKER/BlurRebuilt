#ifndef DISPLAY_OCCLUSION_GCM_GCM_OCCLUSIONQUERY_H
#define DISPLAY_OCCLUSION_GCM_GCM_OCCLUSIONQUERY_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Occlusion/OcclusionQueryTypes.h"

namespace Display
{
namespace GCM
{

//! C_OcclusionQuery
    
class C_OcclusionQuery
{
public:

	C_OcclusionQuery();
	~C_OcclusionQuery();

	const bool IsCreated() const;

	bool Create();
	void Destroy();

	void Begin();
	void End();

	const E_OcclusionQueryResult Visible() const;
	const E_OcclusionQueryResult Visible(U32& visible) const;
	const E_OcclusionQueryResult Visible(F32& visible) const;

private:

	U32 m_CurrIndex;
};

}; // namespace GCM
}; // namespace Display

#include "Display/Occlusion/GCM/GCMOcclusionQuery.inl"

#endif // DISPLAY_OCCLUSION_GCM_GCM_OCCLUSIONQUERY_H
