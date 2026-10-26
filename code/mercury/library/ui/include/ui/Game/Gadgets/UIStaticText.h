#ifndef UISTATICTEXT_H
#define UISTATICTEXT_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/Game/Gadgets/Buttons.h"
#include "ui/Graphics/Font/Font.h"
#include "ui/Graphics/Font/FontStyle.h"
#include "ui/Graphics/Font/TextEffect.h"
#include "ui/Game/UiXt.h"

#include <btl/Containers/containers.h>

namespace UI
{

// Shader interface (compressed)
class C_FontInterfaceParam
{
public:
	const bool	Setup( lua_State* pState );
	const U32	GetEffectParam( ) { return *((U32*)&m_Data); }

public:
	typedef struct
	{
		U32		m_OutlineI		: 4;
		U32		m_OutlineA		: 4;
		U32		m_OutlineTex	: 1;

		U32		m_PipingI		: 4;
		U32		m_PipingA		: 4;
		U32		m_PipingTex		: 1;

		U32		m_CentreI		: 4;
		U32		m_CentreA		: 4;
		U32		m_CentreTex		: 1;

		U32		m_Flags			: 5;
	} T_Data;

	T_Data		m_Data;

};

class UIStaticText : public C_GadgetBase
{
public:
	class C_Cache : public C_FontPreRender
	{
	public:
		C_Cache() : m_Valid( false ) 		{ }

		virtual void		ProcessStack( C_FontSpriteStack& stack );

		void				SetAlpha( const C_UISpriteColours& col );
		void				SetColour( const C_UISpriteColours& col );

		void				Invalidate()	{ m_Valid = false; }

		const bool			IsValid()		{ return m_Valid && m_Stack.Size() > 0; }
		C_FontSpriteStack&	Stack()			{ return m_Stack; }

	private:
		C_FontSpriteStack	m_Stack;
		bool				m_Valid;				
	};

public:
	UIStaticText( ) : m_VariableName( XtSz8(NULL) ), m_pDirectText( NULL ), m_pCache( NULL ) { }

	DECLARE_GADGET( E_UIButtonType_StaticText, "TEXT" )

	virtual ~UIStaticText( );
	
	virtual void			Init();
	virtual void			Setup( lua_State* pState );
	virtual void			Render( );
	virtual void			FrameUpdate( );

	void					ChangeSize( const F32 fX, const F32 fY );
	void					ChangeVariableName( const char* new_name );
	void					SetupMouseBoundingBox( );

	void					ResetTextPrint();
	void					SkipTextPrint();

	void					SetDirectText( const WCHAR* text )				{ m_pDirectText = text; }
	void					SetCache( C_Cache* pCache )						{ BIZ_ASSERT( !m_EnableCache ); m_pCache = pCache; } // Note : *should not* be allowed if caching was already enabled, as it will wipe the pointer

							/// Returns the size in pixels that the gadget will take up based on the text string
	const F32				GetStringLength( const bool bUpdateLengths = false );
	const F32				GetStringHeight();

protected:
	virtual const WCHAR*	GetString();

public:
	void					ChangeText( const C_TextStringID& number );
	void					ChangeText( const char* string );
	void					DoMaintainWidth();
	void					DoCalcStringSize( const WCHAR* pStringOverride = NULL, const bool bForceCalculate = false );
	void					DoTruncateString();

	// Exposed for XT
	C_FontStylePasses		m_FontStylePasses;	// new multiple pass info

	XtSz8					m_VariableName;
	XtF32					m_PrintDelayTime;
	XtLocId					m_TextID;
	bool					m_CalcStringSize;
	bool					m_MaintainWidth;
	bool					m_Truncate;
	bool					m_EnableCache;

	C_TextStringID			m_Text;

	E_FixedSpaceType		m_eFixedSpaceType;

protected:
	C_TextEffect*			m_pEffect;
	C_Cache*				m_pCache;

#ifdef PLATFORM_PC
	C_UIColour				m_MouseOverToColour;
	F32						m_fMouseOverEffectTime;
	F32						m_fMouseOverTimer;
#endif

	bool					m_PrintText;
	XtF32					m_PrintTimer;
	S32						m_PrintAmount;
	S32						m_TruncateAmount;
	F32						m_DrawWidth;
	F32						m_DrawHeight;
	const WCHAR*			m_pDirectText;

	F32						m_fOriginalScaleX;
};

}; // namespace UI

#endif // UISTATICTEXT_H
