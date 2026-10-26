#ifndef EFFECT_POSTPROCESS_DOFBASE_H
#define EFFECT_POSTPROCESS_DOFBASE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include <Btl/Containers/Vector.h>

namespace PostProcess
{

struct C_DepthOfFieldSettings
{
	C_DepthOfFieldSettings() : m_fAperture(0.0f), m_fPlaneInFocus(0.0f) {}

	F32	m_fAperture;
	F32	m_fPlaneInFocus;
};

class C_DoFBase
{
public:

	C_DoFBase();
	~C_DoFBase();

	F32 GetPlaneInFocus(const S32 iViewIndex) const;
	F32 GetAperture(const S32 iViewIndex) const;

	void SetDepthOfField(const S32 iViewIndex, const F32 fPlaneInFocus, const F32 fAperture);

protected:

	Btl::Vector<C_DepthOfFieldSettings> m_ViewportSettings;

};

extern Debug::Float g_debugDoFFocalLength;
extern Debug::Float g_debugDoFMinCoC;

}; // namespace PostProcess

#endif // EFFECT_POSTPROCESS_DOFBASE_H
