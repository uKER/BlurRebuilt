//=============================================================================
// CameraDollyZoomSpecialFX.h
// 29/01/09
//=============================================================================
#ifndef _CAMERA_DOLLY_ZOOM_SPECIALFX_H_
#define _CAMERA_DOLLY_ZOOM_SPECIALFX_H_

#include <SpecialFX/SpecialFX.h>
#include <btl/Containers/list.h>

namespace AmaxCam
{

class C_CameraDollyZoomSpecialFX : public SpecialFX::C_BaseSpecialFXComponent
{
public:

	class C_DollyZoomOutput : public Btl::ListElementDouble<C_DollyZoomOutput>
	{
	public:
		C_DollyZoomOutput();
		F32		m_Fov;
		S32		m_ViewportID;
	private:

	};

	typedef Btl::List<C_DollyZoomOutput, Btl::ListParameters<Btl::DoubleEndedList, Btl::MaintainedCounter> >	ActiveDollyZoomList;


	XtArray<SpecialFX::C_HermiteKeyFrame>		m_FovGraph;
	F32											m_Time;
	virtual SpecialFX::I_SpecialFXComponentInstance* CreateInstance(const Sys::I_ContextProvider* pContextProvider, const ResourceUtils::I_ResourceBag* pResourceBag) const;

	static const ActiveDollyZoomList& GetActiveDollyZoomList();
};


} // End of namespace
#endif //_CAMERA_DOLLY_ZOOM_SPECIALFX_H_
