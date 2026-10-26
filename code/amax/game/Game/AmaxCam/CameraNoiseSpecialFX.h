//=============================================================================
// CameraNoiseSpecialFX.h
// 02/11/09
//=============================================================================
#ifndef _CAMERA_NOISE_SPECIALFX_H_
#define _CAMERA_NOISE_SPECIALFX_H_

#include <SpecialFX/SpecialFX.h>
#include <btl/Containers/list.h>

namespace AmaxCam
{

class C_CameraNoiseSpecialFX : public SpecialFX::C_BaseSpecialFXComponent
{
public:

	class NoiseParams
	{
	public:
		XtF32 m_XScale;
		XtF32 m_YScale;
		XtF32 m_ZScale;
		XtArray<SpecialFX::C_HermiteKeyFrame>		m_XIntensityCurve;
		XtArray<SpecialFX::C_HermiteKeyFrame>		m_YIntensityCurve;
		XtArray<SpecialFX::C_HermiteKeyFrame>		m_ZIntensityCurve;
	};

	class C_NoiseOutput : public Btl::ListElementDouble<C_NoiseOutput>
	{
	public:
		C_NoiseOutput();
		//Maths::C_Quaternion	m_Orientation;
		Maths::HVec3 m_Orientation;	
		Maths::HVec3 m_Position;

		S32		m_ViewportID;
	private:

	};

	typedef Btl::List<C_NoiseOutput, Btl::ListParameters<Btl::DoubleEndedList, Btl::MaintainedCounter> >	ActiveNoiseList;




	virtual SpecialFX::I_SpecialFXComponentInstance* CreateInstance(const Sys::I_ContextProvider* pContextProvider, const ResourceUtils::I_ResourceBag* pResourceBag) const;

	static const ActiveNoiseList& GetActiveNoiseList();

	F32						m_Time;
	F32						m_Frequency;

	NoiseParams				m_TranslationParams;
	NoiseParams				m_RotationParams;

};


} // End of namespace
#endif //_CAMERA_NOISE_SPECIALFX_H_
