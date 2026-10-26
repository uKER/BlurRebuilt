#ifndef UILEGAL_H
#define UILEGAL_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/Game/Gadgets/Buttons.h"
#include "ui/Graphics/Font/Font.h"
#include "ui/Graphics/Font/FontStyle.h"

#include <btl/Containers/containers.h>

namespace UI
{

class UILegal : public C_GadgetBase
{
public:
	enum E_DisplayStyle
	{
		E_DisplayStyle_Scroller = 0,
	};
public:
	DECLARE_GADGET( E_UIButtonType_Legal, "LEGAL" )

public:
	void							Init();
	void							Setup( lua_State *pState );
	bool							MessageHandler( const Messaging::C_MessageBase* pMessage );
	void							FrameUpdate();
	void							Render();
	void							Exit();

	
public:
	class C_Text
	{
	public:
		class C_Line
		{
		public:
			C_Line( ) : m_Chars(NULL)
			{
			}
			~C_Line( )
			{
				if(m_Chars)
				{
					delete [] m_Chars;
					m_Chars = NULL;
				}
			}
		public:
			enum E_Type
			{
				E_Type_MainHeading=0,
				E_Type_Heading1,
				E_Type_Heading2,
				E_Type_Text,
				E_Type_Graphics,
				E_Type_Count
			};

			E_Type			m_Type;
			WCHAR*			m_Chars;
			C_TextStringID	m_StringID;
		};

		C_Text() : m_Line(NULL), m_Built(false)
		{
		}

		void Destroy( )
		{
			if(!m_Built)
				return;

			MEMORY_TRACKER_PUSH("ui.legal");
			if(m_Line)
			{
				delete [] m_Line;
				m_Line = NULL;
			}
			m_Built = false;
			MEMORY_TRACKER_POP("ui.legal");
		}

		~C_Text()
		{
			if(m_Built)
				Destroy();
		}

		static C_Text&				Instance()						{ return m_sInstance; }
		void						Load( const char *filename );
		const U32					LineCount( )					{ return m_LineCount; }
		C_Line&						Line(const U32 index )			{ assert(index<m_LineCount); return m_Line[index]; }
		const bool					Built( )						{ return m_Built; }

	private:
		static C_Text				m_sInstance;
		U32							m_LineCount;
		C_Line*						m_Line;
		bool						m_Built;
	};
private:
	void	RenderScroller( );

// Exposed for Sorcery
public:
	XtArray< XtSz8 >				m_ColourStyleName;
	XtArray< C_FontStylePasses >	m_Style;
	XtArray< XtF32 >				m_Height;
	XtArray< XtF32 >				m_PreCr;
	XtArray< XtF32 >				m_PostCr;
	XtF32							m_ScrollSpeed;
	XtF32							m_StartPos;
	XtF32							m_FadeBottom;
	XtF32							m_FadeLength;

private:


	enum E_ScrollSpeed
	{
		E_ScrollSpeed_DFast = 0,
		E_ScrollSpeed_DSlow,
		E_ScrollSpeed_Off,
		E_ScrollSpeed_USlow,
		E_ScrollSpeed_UFast,
	};

	U32								m_AutoScroll;
	UI::C_UISpriteColours			m_Colour[ C_Text::C_Line::E_Type_Count ];

	Btl::Vector< U8 >				mNumLines;	// there shouldn't be any lines longer than 255?????												

	E_DisplayStyle					m_DisplayStyle;
	F32								m_Offset;
	F32								m_OriginalScrollSpeed;
};

}; // namespace UI

#endif
