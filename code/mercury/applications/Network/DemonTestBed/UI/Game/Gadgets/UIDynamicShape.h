
#ifndef UI_UIDYNAMIC_SHAPE_H
#define UI_UIDYNAMIC_SHAPE_H


#include "UiManager.h"
#include "ui/Game/Gadgets/UIStaticShape.h"

namespace UI
{


class UIDynamicShape : public UIStaticShape
{
public:
	DECLARE_GADGET( E_ButtonType_DynamicShape, "DYNAMIC_SHAPE" );

	UIDynamicShape();
	~UIDynamicShape();

public:
	virtual void Init();
	virtual void Setup();
	virtual void Render();
	virtual void FrameUpdate();

private:
	S32		m_CurrentLayer;
	S32		m_CurrentGroup;
	S32		m_CurrentElement;
};


} // namespace 

#endif // UI_UIDYNAMIC_SHAPE_H
