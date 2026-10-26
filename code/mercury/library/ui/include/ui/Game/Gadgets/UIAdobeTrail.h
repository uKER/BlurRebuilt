
#ifndef UI_ADOBE_WAVE_H
#define UI_ADOBE_WAVE_H

#include "Ui/Game/Gadgets/UIStaticShape.h"

namespace UI
{

/*---------------------------------------------------------------------------------------
	Class : UIAdobeTrail

	Desc :	Gadget to manipulate path vertices giving a wave effect.
---------------------------------------------------------------------------------------*/
class UIAdobeTrail : public UIStaticShape
{
public:
	DECLARE_GADGET( E_UIButtonType_AdobeTrail, "ADOBE_TRAIL" )

	void		Setup( lua_State* pState );

	void		Render();

	void		RenderStroke( const C_AdobeShape* pShape, const Maths::HVec3& vPos, const F32 fSizeX, const F32 fSizeY, const TransformIndex uColourIndex );

	void		Frequency( const F32 fFrequency )
				{
					m_WaveFrequency = fFrequency;
				}

	void		Intensity( const F32 fIntensity )
				{
					m_WaveIntensity = fIntensity;
				}

	void		Speed( const F32 fSpeed )
				{
					m_WaveSpeed = fSpeed;
				}

public:
	F32		m_WaveSpeed;
	F32		m_WaveFrequency;
	F32		m_WaveIntensity;

private:
	F32		m_Time;
};

}; // namespace UI

#endif // UI_ADOBE_WAVE_H