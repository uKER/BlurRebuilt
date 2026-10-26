#ifndef DEBUGHUDMENU_H
#define DEBUGHUDMENU_H

#ifdef _MSC_VER
#pragma once
#endif

#include "DebugSystems/Frame.h"

namespace Render { namespace Font { class C_FontRenderer;}}

namespace DebugSystems
{
class C_Frame;

//Interface to control the DebugHUDMenu
class I_Controller
{
public:
	// Debounced movement (preferably with delay & repeat)
	virtual const bool 		MoveUp( ) = 0;
	virtual const bool 		MoveDown( ) = 0;
	virtual const bool 		MoveLeft( ) = 0;
	virtual const bool 		MoveRight( ) = 0;
	// Debounced selection
	virtual const bool 		Confirm( ) = 0;
	virtual const bool 		Cancel( ) = 0;
	// Debounce switch					   
	virtual const bool 		Open( ) = 0;
	virtual const bool 		Close( ) = 0;
	//Delta position
	virtual const bool		PositionMove(F32& deltaX, F32& deltaY) = 0;
};

/// Skin for defining the render style of the DebugMenu (default is a kinda Windows Eggplant, yuk!)
class C_DebugHUDMenuSkin
{	
public:
	C_DebugHUDMenuSkin( ) { Reset( );}

private:
	void Reset( );

public:
	/// Note menu is centred on the X but place on the Y
	Maths::HVec2			m_Position;
	Maths::HVec2			m_TitleFontScale;
	Maths::HVec2			m_NormalFontScale;

	/// Note : Width = (largest 'item text' * font size) + (largest 'value text' * font size) + ((outline + border) * 2)
	F32						m_MaxHeight;

	/// Set to 0,0 for no border
	Maths::HVec2			m_BorderSize;
	/// Set to 0,0 to not draw any outline
	Maths::HVec2			m_OutlineSize;
	/// Set to 0,0 to not draw and margins
	Maths::HVec2			m_MarginSize;

	Maths::RGBA				m_SelctionColour;
	Maths::RGBA				m_MarginColour;

	Maths::RGBA				m_SelectedFontColour;
	Maths::RGBA				m_DeselectedFontColour;
	Maths::RGBA				m_TitleFontColour;

	Util::C_String			m_FontName;

	C_Frame*				m_pFrame;
};


class C_MenuHUDItem
{
public:
	C_MenuHUDItem( const Util::C_String& name ) : m_Name( name ) { Clear(); }
	virtual ~C_MenuHUDItem();

	void							AddChild( C_MenuHUDItem* pChild);
	Util::C_String&					Name( ) { return m_Name;}
	virtual Util::C_String			Value( ) { return (m_pFirstChild) ? Util::C_String("=>") : Util::C_String("");};

	virtual void 					Increase(const U32 /*keydown*/) {};
	virtual void 					Decrease(const U32 /*keydown*/) {};
	virtual bool 					Forwards() { return true;}; //Returns if has to navigate to their children
	virtual bool					BackColour(Maths::RGBA& backColour) { return false;}; 

private:
	void							Clear( );

public:
	C_MenuHUDItem *					m_pParent;
	C_MenuHUDItem *					m_pNext;
	C_MenuHUDItem *					m_pPrev;
	C_MenuHUDItem *					m_pFirstChild;

private:
	Util::C_String					m_Name;

	friend class C_DebugHUDMenu;
};

class C_DebugHUDMenu
{
public:
	C_DebugHUDMenu(I_Controller* pController, const C_DebugHUDMenuSkin PCREF debugHUDMenuSkin);
	~C_DebugHUDMenu();

	void							Update( const Maths::C_TimeS& timeStep );
	void							Render( );

	const bool						Active( );
	void							Active( const bool on );

	void							Current( C_MenuHUDItem *pCurrent )							{ m_pCurrentMenu = pCurrent; }
	C_MenuHUDItem *					Current( )												{ return m_pCurrentMenu; }

	void							Root( C_MenuHUDItem *pRoot )								{ m_pRootMenu = pRoot; }
	C_MenuHUDItem *					Root( )													{ return m_pRootMenu; }

	const U32						KeyDown( )												{ return m_Keydown; }

	void							RefreshMenuSize( );

	static C_DebugHUDMenu*			GetInstance();

	bool							RemoveItem(C_MenuHUDItem* pRemoveItem);

private:
	void							RebuildDebugVars( );
	void							CalcMenuExtentsRecursive( F32& maxName, F32& maxValue, C_MenuHUDItem *pItem );
	void							CalcMenuExtents( F32& maxName, F32& maxValue, C_MenuHUDItem *p );
	void							UpdateDebugVarMenuPosition( );
	S32								FindSeparator(Util::C_String &string);
	void							UpdatePosition();
	
	void 							Up();
	void 							Down();
	void 							Forwards();
	void							Backwards();

private:

	C_DebugHUDMenuSkin				m_Skin;
	C_Frame*						m_pFrame;

	Render::Font::C_FontRenderer*	m_pSelectedFont;
	Render::Font::C_FontRenderer*	m_pDeselectedFont;
	Render::Font::C_FontRenderer*	m_pTitleFont;

	C_MenuHUDItem *					m_pCurrentMenu;
	C_MenuHUDItem *					m_pRootMenu;
	Maths::HVec2					m_Position;
	Maths::HVec2					m_NamePos;
	Maths::HVec2					m_ValuePos;
	Maths::HVec2					m_TitlePos;
	F32								m_NameWidth;
	F32								m_ValueWidth;
	bool							m_Active;
	U32								m_Keydown;

	bool						    m_MarginBlendMode;
	bool						    m_HighlightBlendMode;

	I_Controller*					m_pController;
};

class C_ResumeMenuItem : public C_MenuHUDItem
{
public:
	C_ResumeMenuItem( C_DebugHUDMenu* pDebugHUDMenu ) : C_MenuHUDItem("Resume") {m_pDebugHUDMenu = pDebugHUDMenu;};

	virtual bool 					Forwards(  );

private:
	C_DebugHUDMenu* m_pDebugHUDMenu;
};

class C_DebugVarMenuItem : public C_MenuHUDItem
{
public:
	C_DebugVarMenuItem( const Util::C_String name, Debug::VarBase* pOption );
	virtual ~C_DebugVarMenuItem( );
	
	virtual void 					Increase( const U32 keydown);
	virtual void 					Decrease( const U32 keydown);
	virtual bool 					Forwards( );

	virtual Util::C_String			Value( );
	virtual bool					BackColour(Maths::RGBA& backColour); 

private:
	

private:
	Debug::VarBase*					m_pOption;
};

class C_DebugVarMenuColourHelperItem : public C_MenuHUDItem
{
public:
	C_DebugVarMenuColourHelperItem( const Util::C_String name, Debug::Colour* pColourOption, const U32 channel);
	
	virtual void 					Increase( const U32 keydown );
	virtual void 					Decrease( const U32 keydown );
	virtual bool					Forwards();

	virtual Util::C_String			Value( );
	virtual bool					BackColour(Maths::RGBA& backColour); 

private:
	Debug::Colour*					m_pColourOption;
	U32								m_Channel; //RGBA
};

class C_DebugVarMenuVectorHelperItem : public C_MenuHUDItem
{
public:
	C_DebugVarMenuVectorHelperItem( const Util::C_String name, Debug::Vector* pVectorOption, const U32 index);
	
	virtual void 					Increase( const U32 keydown );
	virtual void 					Decrease( const U32 keydown );
	virtual bool					Forwards();

	virtual Util::C_String			Value( );

private:
	Debug::Vector*					m_pVectorOption;
	U32								m_Index; //XYZ
};

} // namespace DebugSystems

#endif
