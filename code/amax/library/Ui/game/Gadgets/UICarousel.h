#ifndef UI_CAROUSEL_H
#define UI_CAROUSEL_H

#include <Ui/Game/Gadgets/BaseUIVariableSelect.h>
#include <Ui/UIGameDefines.h>
#include "Ui/UiManager.h"

namespace UI
{

class UICarousel : public I_SelectableGadgetBase
{
public:
	DECLARE_GADGET( E_UIGameButtonType_Carousel, "CAROUSEL" );

public:
	// Gadget interface
	virtual void Init();
	virtual void Setup( lua_State* pState = NULL );
	virtual void PostInit();
	virtual void FrameUpdate();
	virtual bool MessageHandler(const Messaging::C_MessageBase *pMessage); // from C_MessageHandler

	// Selectable gadget interface
	virtual const bool			SetCurrentItemByIndex( const S32 index );
	virtual const bool			SetCurrentItemByValue( const F64 value );

	virtual const S32			GetCurrentItemIndex() const;
	virtual const F64			GetCurrentItemValue( const U32 arg = 0 ) const;

	void						BranchScan();

public:
	XtI32				m_NumItems;
	XtF32				m_FoldAppear;

private:
	void				UpdatePositions();
	void				DepthSort();
	void				MoveRight();
	void				MoveLeft();

private:
	Btl::Vector<S32>			m_SortedRef;
	Btl::Vector<S32>			m_NodeRef;
	Btl::Vector<Maths::HVec3>	m_NodePositions;
	Btl::Vector<F32>			m_NodeRotations;
	ButtonArray					m_Children;
	F32							m_Angle;
	F32							m_AngleVel;
	S32							m_SelectedItem;
	F32							m_MovedTime;
	F32							m_ExpandSize;
};


} // namespace 

#endif // UI_CAROUSEL_H
