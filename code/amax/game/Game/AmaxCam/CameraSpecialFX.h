#ifndef CAMERASPECIALFX_H
#define CAMERASPECIALFX_H

#include <SpecialFX/SpecialFX.h>

namespace AmaxCam
{

class C_CameraSpecialFX : public SpecialFX::C_BaseSpecialFXComponent
{
public:
	XtF32										m_Time;
	XtArray<SpecialFX::C_HermiteKeyFrame>		m_OffsetXGraph;
	XtArray<SpecialFX::C_HermiteKeyFrame>		m_OffsetYGraph;
	XtArray<SpecialFX::C_HermiteKeyFrame>		m_OffsetZGraph;
	XtArray<SpecialFX::C_HermiteKeyFrame>		m_OffsetPitchGraph;
	XtArray<SpecialFX::C_HermiteKeyFrame>		m_OffsetYawGraph;
	XtArray<SpecialFX::C_HermiteKeyFrame>		m_OffsetRollGraph;
	XtArray<SpecialFX::C_HermiteKeyFrame>		m_OffsetFovGraph;

	virtual SpecialFX::I_SpecialFXComponentInstance* CreateInstance(const Sys::I_ContextProvider* pContextProvider, const ResourceUtils::I_ResourceBag* pResourceBag) const;
};


} // End of namespace
#endif
