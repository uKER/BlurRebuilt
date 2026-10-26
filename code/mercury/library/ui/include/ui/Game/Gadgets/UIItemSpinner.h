

#ifndef UI_ITEMSPINNER_H
#define UI_ITEMSPINNER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/Game/Gadgets/Buttons.h"


namespace UI
{

enum E_ItemDisplayMode
{
	E_ItemDisplayMode_Horizontal = 0,
	E_ItemDisplayMode_Vertical,
	E_ItemDisplayMode_Stacked,
};


struct Slot
{
	C_GadgetBase* pSlot;

	Btl::Vector< C_GadgetBase*,  Btl::VectorParameters_Dynamic<> > AttachedList;
};


class UIItemSpinner : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIButtonType_ItemSpinner, "ITEM_SPINNER" )

public:
	void Init();
	void Setup( lua_State *pState );
	bool MessageHandler( const Messaging::C_MessageBase* pMessage );
	void FrameUpdate();
	void Render();
	void Exit();

// Functionality for managing the container.
public:
	S32 AddSlot();

	void AddItem( const S32 slot, C_GadgetBase* pData );

	void RemoveLastItem()
	{
		m_LastSlot--;
		m_SlotList.PopBack();
	}

public:
	// Exposed for XT
	F32		m_MoveSpeed;
	F32		m_ScaleSpeed;
	F32		m_Spacing;
	S32		m_WrapSlotList;
	S32		m_SlotMoveDelay;

	F32		m_ItemScale; // Remove once effects work.

private:
	void ReorderRenderSlots();

	void UpdateSlot( S32 slot, F32 x, F32 y, F32 z, F32 scale, bool useSlotList = true );

private:
	static const U32 m_MaxSlots = 12;

	S32		m_CurrentSlot;
	S32		m_LastSlot;
	S32		m_ChangeSlot;

	E_ItemDisplayMode	m_DisplayMode;

	Btl::Vector< Slot*,  Btl::VectorParameters_Static< m_MaxSlots > >	m_SlotList;
	Btl::Vector< Slot*, Btl::VectorParameters_Static< m_MaxSlots > >	m_RenderList;
};

};// namespace UI

#endif // UI_ITEMSPINNER_H
