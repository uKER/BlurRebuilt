
#ifndef UI_ADOBE_ANIMATION_H
#define UI_ADOBE_ANIMATION_H

#include "ui\Game\Gadgets\UIStaticShape.h"
#include "ui\Game\Gadgets\UIAdobeTrail.h"

namespace UI
{

class UIAdobeAnimation : public UIAdobeTrail //UIStaticShape
{
public:
	DECLARE_GADGET( E_UIButtonType_AdobeAnimation, "ADOBE_ANIMATION" )

	virtual void		Init();
	virtual void		Setup( lua_State* pState );
	virtual void		FrameUpdate();

public:
	F32					m_AnimationTime;

private:
	static const U32	m_MaxLayers = 10;
	
	U32					m_CurrentFrame[m_MaxLayers];
	F32					m_LayerTimer[m_MaxLayers];
};

}; // namespace

#endif // UI_ADOBE_ANIMATION_H