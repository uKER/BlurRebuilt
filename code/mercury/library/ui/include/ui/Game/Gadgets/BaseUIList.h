
#ifndef UI_BASE_LIST_H
#define UI_BASE_LIST_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui\Game\Gadgets\BaseUIVariableSelect.h"

namespace UI
{

/*---------------------------------------------------------------------------------------
	Forward declarations
---------------------------------------------------------------------------------------*/
class UIScrollBar;


/*---------------------------------------------------------------------------------------
	Class : BaseUIList

	Desc : Base class for rendering a list of items. Derived classes should implement the
		   RenderItem() function so the list can be displayed. Clipping will be handled
		   for free along with item scrolling, moving and a scrollbar is required.
---------------------------------------------------------------------------------------*/
class BaseUIList : public I_SelectableGadgetBase
{
public:
								BaseUIList();

	virtual void 				Init();
	virtual void 				PostInit();
	virtual void 				Setup( lua_State* pState );
	virtual bool				MessageHandler( const Messaging::C_MessageBase* pMessage );
	virtual void 				FrameUpdate();
	virtual void 				Render();

								/// Scroll the list by the amount.
	virtual void				Scroll( const F32 amount );

								/// Scroll untill the current item is in view.
	virtual void				ScrollToCurrentItem( const bool bSnap );

								/// Returns true if their are enough items to scroll.
	const bool					CanScroll() const;

								/// Sets the currently selected item.
	virtual const bool			SetCurrentItemByIndex( const S32 index )
								{
									if ( index >= 0 && index < GetNumberOfItems() )
									{
										m_CurrentItem = index;
										return true;
									}

									return false;
								}

	virtual const bool			SetCurrentItemByValue( const F64 value )
								{
									// TODO
									return false;
								}

								/// Returns the currently selected item.
	virtual const S32			GetCurrentItemIndex() const
								{
									if ( GetNumberOfItems() )
										return m_CurrentItem;
									return -1;
								}

								/// Get value of current item
	virtual const F64			GetCurrentItemValue( const U32 arg = 0 ) const
								{
									return 0;
								}


								/// Height of each item row.
	const F32					GetRowHeight() const
								{
									return m_RowHeight;
								}


/*---------------------------------------------------------------------------------------
	User must supply the implementation of these functions. Cant make method abstract due to xt.
---------------------------------------------------------------------------------------*/

								/// Returns the number of items
	virtual S32					GetNumberOfItems() const; // = 0;

								/// Function defining how each item should be rendered.
	virtual void 				RenderItem( const F32 fX, const F32 fY, const U32 uItem, const Maths::HVec3& vRelativePos ); //= 0;

/*---------------------------------------------------------------------------------------
	Privite functions
---------------------------------------------------------------------------------------*/
private:
	void						SetupStencilBufferWrite( S32 ref = -1 );
	void						SetupStencilBufferRead( S32 ref = -1 );
	void						MoveUp();
	void						MoveDown();

	const S32					CurrentItem();

/*---------------------------------------------------------------------------------------
	Public data exposed for xt
---------------------------------------------------------------------------------------*/
public:
	F32							m_RowHeight;			/// <@
	F32							m_RowPadding;			/// <@
	F32							m_ScrollAmount;			/// <@
	F32							m_ScrollSpeed;			/// <Scroll movement speed

	Padding						m_Padding;				/// <Contents padding.

	bool						m_bAllowScrollingOnly;

/*---------------------------------------------------------------------------------------
	Protected data
---------------------------------------------------------------------------------------*/
protected:
	Display::C_StencilState		m_StencilState;			/// <Stencil buffer state.

	S32							m_CurrentItem;			/// <@
	S32							m_SoundEffectMove;		/// <@

	F32							m_PosX;					/// <Item start posX
	F32							m_PosY;					/// <Item start posY
	F32							m_DestX;				/// <Item required posX
	F32							m_DestY;				/// <Item required posY

	UIScrollBar*				m_pScrollBar;			/// <Attached scrollbar

	bool						m_FlippedList;

	/// Buffer for holding text that you want to pass to the data finder.
	static const U32			m_BufferSize = 128;
	char						m_DataFinderBuffer[m_BufferSize];

	C_RenderState				m_StencilRenderState;
};

}; // namespace UI

#endif // UI_BASE_LIST_H
