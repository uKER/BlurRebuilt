#ifndef BASEUIVARIABLESELECT_H
#define BASEUIVARIABLESELECT_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/Game/Gadgets/Buttons.h"


#include "ui/Graphics/Font/Font.h"
#include "ui/Graphics/Font/FontStyle.h"
#include "ui/Graphics/Localisation/loc.h"
#include "ui/Game/Gadgets/UIStaticText.h"
#include "ui/Game/UIScreenControl.h"

#include <btl/Containers/containers.h>

namespace UI
{


/*--------------------------------------------------------------------------------------
	Item that the variableselect gadget stores.
--------------------------------------------------------------------------------------*/
class C_VariableItem
{
public:
	C_VariableItem()
		: m_Text( L"" ), m_HelpText( L"" ), m_bUnlocked( true ), m_bVisible( true ), m_Value( 0 )
	{
		m_Timer.Init( UITimer::E_Type_Stop, 0.0f, 0.0f );
	}

public:
	C_WideString128		m_Text;
	C_WideString64		m_HelpText;

	S32					m_Value;
	bool				m_bUnlocked;
	bool				m_bVisible;
	UITimer				m_Timer;
};

// Selectable gadget interface
class I_SelectableGadgetBase : public C_GadgetBase
{
public:
	virtual const bool			SetCurrentItemByIndex( const S32 index ) = 0;
	virtual const bool			SetCurrentItemByValue( const F64 value ) = 0;

	virtual const S32			GetCurrentItemIndex() const = 0;
	virtual const F64			GetCurrentItemValue( const U32 arg = 0 ) const = 0;
};

/*--------------------------------------------------------------------------------------
	Generic base class that implements the functionality for controlling a list of items.
--------------------------------------------------------------------------------------*/
class BaseUIVariableSelect : public I_SelectableGadgetBase
{
public:
							BaseUIVariableSelect(){}
	virtual					~BaseUIVariableSelect(){}

public:
	/// Type def for ease of use.
	typedef Btl::Vector< C_VariableItem > TVariableList;


// Main functions
public:
	virtual void			Init();
	virtual void			Setup( lua_State* pState );
	virtual bool			MessageHandler( const Messaging::C_MessageBase *pMessage );
	virtual void			Render();

							/// Flags the lists as empty and resets all data.
	virtual void			ClearItems()
							{
								for ( S32 i = 0; i < m_Items.Size(); i++ ) 
									m_Items[ i ].m_bUnlocked = true;

								m_CurrentItem = 0;
								m_Items.Clear();
							}

							/// Sets the current item to 0
	virtual void			ResetCurrentItem()
							{
								m_CurrentItem = 0;
							}

							/// Returns a paramitized value for the current item ( 0.0f - 1.0f )
	virtual const			F32 GetDelta() const
							{ 
								if( m_Items.Size() <= 1 ) // Incase divide by zero. Not good :-s
									return 0.0f;

								return (F32)m_CurrentItem / (F32)(m_Items.Size()-1);
							}

							/// Adds a new item to the list assigning the value as its id.
	virtual void			AddItem( const WCHAR* pText, U32 value );
			
							/// Adds helper text to the last added item on the list.
	void 					AddItemHelp( const WCHAR* pText );

							/// Moves current item onto the next one. Returns true if current item changed.
	bool					SetNextItem();

							/// Moves current item onto the previous one. Returns true if current item changed.
	bool					SetPreviousItem();

							/// Sets the unlocked flag of the item's id matching the value.
	void					SetItemUnlocked( S32 value, bool unlocked );

							/// Sets the unlocked flag of the last item added.
	void					SetItemUnlocked( bool unlocked )
							{
								if ( m_Items.Size() > 0 )
									SetItemVisible( m_Items[m_Items.Size()-1].m_Value, unlocked );
							}

							/// Sets the visible flag of the item's id matching the value.
	void					SetItemVisible( S32 value, bool visible );

							/// Sets the visible flag of the last item added.
	void					SetItemVisible( bool visible )
							{
								if ( m_Items.Size() > 0 )
									SetItemVisible( m_Items[m_Items.Size()-1].m_Value, visible );
							}

							/// Returns the unlocked state of the current item.
	bool 					IsCurrentItemUnlocked() const
							{
								return ( m_Items.Size() > 0 ) ? m_Items[ m_CurrentItem ].m_bUnlocked : false;
							}

	bool					IsCurrentItemVisible() const
							{ 
								return ( m_Items.Size() > 0 ) ? m_Items[ m_CurrentItem ].m_bVisible : false;
							}
	
							/// Sets the item by an index
	virtual const bool		SetCurrentItemByIndex( const S32 index );

							/// Sets the item with the given value as the current item.
	virtual const bool		SetCurrentItemByValue( const F64 value );

							/// Returns the value assigned to the current item.
	virtual const F64		GetCurrentItemValue( const U32 arg = 0 ) const
							{
								return (F64)( m_Items.Size() > 0 ) ? m_Items[ m_CurrentItem ].m_Value : 0;
							}

							/// Returns the current item index.
	virtual const S32		GetCurrentItemIndex() const
							{
								return ( m_Items.Size() > 0 ) ? m_CurrentItem : -1;
							}

							/// Set the current pad index.
	void					SetPad( S32 index )	
							{ 
								m_Pad = index; 
							}

							/// @
	void					SetupStyles( lua_State* pState );

	TVariableList&			GetItemList()
							{
								return m_Items;
							}

protected:
	void					ResolveList();

	const WCHAR*			GetString( S32 item ) const
							{
								// Check string and array access is valid.
								if( gUIScreenControl.InvalidStrings() || item < 0 || item >= m_Items.Size() )
									return L"Invalid String";

								return m_Items[item].m_Text.Chars();
							};
// Members exposed for sorcery.
public:
	class C_Item
	{
	public:
		C_Item( ) : m_OnNextCmd( XtSz8( NULL ) ), m_NameString( XtSz8( NULL ) ) { }

	public:
		XtLocId		m_NameId;
		XtSz8		m_NameString;
		XtLocId		m_HelpId;
		XtSz8		m_HelpString;
		XtI32		m_Value;
		XtSz8		m_OnNextCmd;
	};

public:
	// Auto add items
	XtArray< C_Item >		m_SorceryItems;

	S32						m_Pad;					// Pad controlling to this gadget.
	bool					m_bLoopList;			// Flag to loop the list when min and max items are reached.
	bool					m_bHighlightLocked;		// Flag to allow highlighting of locked items.

	// font styles
	C_FontStylePasses		m_StyleSelected;
	C_FontStylePasses		m_StyleDeselected;
	C_FontStylePasses		m_StyleSelectedUnselectable;
	C_FontStylePasses		m_StyleDeselectedUnselectable;

protected:
	/// Data members.
	TVariableList			m_Items;
	S32						m_CurrentItem;
	S32						m_StartItem;
	S32						m_EndItem;
};

}; // namespace UI

#endif
