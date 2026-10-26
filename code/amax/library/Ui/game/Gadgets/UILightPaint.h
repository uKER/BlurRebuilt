#ifndef UI_UILIGHTPAINT_H
#define UI_UILIGHTPAINT_H

#include "ui/UIGameDefines.h"
#include "ui/Game/Gadgets/Buttons.h"

namespace DynamicEffect { namespace Particle { namespace XtInterface { struct Event; } } }

namespace UI
{

class UILightPaint : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIGameButtonType_LightPaint, "LIGHT_PAINT" )

	virtual void	PostInit();
	virtual void	FrameUpdate();
	virtual void	Exit();

private:
	void			StoreControlPoints();

public:
	XtF32			m_EmitTime;
	XtF32			m_EmitRate;
	XtArray<XtSz8>	m_xtLocatorNames;
	XtArray<XtVec3>	m_xtPositions;
	XtHandle<DynamicEffect::Particle::XtInterface::Event> m_ParticleEffect;

private:
	S32				m_StaticCount;
	S32				m_ControlSize;
	SparkleControl* m_pControls;
};

}; // namespace UI

#endif // UI_UILIGHTPAINT_H