#ifndef _RUMBLE_SPECIALFX_H_
#define _RUMBLE_SPECIALFX_H_

#include <SpecialFX/SpecialFX.h>
#include <btl/Containers/list.h>

namespace SpecialFX
{

class C_RumbleSpecialFX : public SpecialFX::C_BaseSpecialFXComponent
{
public:

	
	class C_RumbleOutput : public Btl::ListElementDouble<C_RumbleOutput>
	{
	public:
				C_RumbleOutput();
		F32		m_LowFreqSpeed;
		F32		m_HighFreqSpeed;
		S32		m_ViewportID;
		S32		m_RumbleTypeID;
	private:

	};

	typedef Btl::List<C_RumbleOutput, Btl::ListParameters<Btl::DoubleEndedList, Btl::MaintainedCounter> >	ActiveRumblesList;


	XtI32										m_RumbleTypeID;
	XtF32										m_Time;

	XtArray<SpecialFX::C_HermiteKeyFrame>		m_LowFreqGraph;
	XtArray<SpecialFX::C_HermiteKeyFrame>		m_HighFreqGraph;

	XtArray<SpecialFX::C_HermiteKeyFrame>		m_LowFreqPulseFreq;
	XtArray<SpecialFX::C_HermiteKeyFrame>		m_HighFreqPulseFreq;

	XtArray<SpecialFX::C_HermiteKeyFrame>		m_LowFreqPulseLength;
	XtArray<SpecialFX::C_HermiteKeyFrame>		m_HighFreqPulseLength;

	virtual SpecialFX::I_SpecialFXComponentInstance* CreateInstance(const Sys::I_ContextProvider* pContextProvider, const ResourceUtils::I_ResourceBag* pResourceBag) const;

	static const ActiveRumblesList& GetActiveRumblesList();
};


} // End of namespace
#endif //_RUMBLE_SPECIALFX_H_
