
#ifndef UI_UIFOCALPOINT_H
#define UI_UIFOCALPOINT_H

#include "ui/UIGameDefines.h"
#include "ui/Game/Gadgets/Buttons.h"

namespace UI
{

class UIFocalPoint : public C_GadgetBase
{
public:
	~UIFocalPoint();

	DECLARE_GADGET( E_UIGameButtonType_FocalPoint, "FOCAL_POINT" )

	virtual void				Init();
	virtual void				Setup( lua_State* pState );
	virtual void				Render();
	virtual void				FrameUpdate();

	static const Maths::HVec3	GetFocalPoint()
								{
									return m_FocalPoint;
								}

	static const bool			FocalPointExists()
								{
									return m_FocalPointExists;
								}

private:
	static Maths::HVec3		m_FocalPoint;
	static bool				m_FocalPointExists;
};

}; // namespace UI

#endif // UI_UIFOCALPOINT_H