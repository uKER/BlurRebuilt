#include "DebugSystemsPch.h"
#include <DebugSystems/DebugHUDMenu.h>
#include <Render/font/Font.h>
#include <Render/debug/DebugRender.h>

namespace DebugSystems
{

static const Util::C_String g_DebugVarsName("Debug Vars");

Debug::Int		gMenuPositionX("GameSys.DebugHUD.DebugMenu.XPosition", 512, Debug::Range(0, 1024));
Debug::Int		gMenuPositionY("GameSys.DebugHUD.DebugMenu.YPosition", 48, Debug::Range(0, 768));
Debug::Float	gMenuSpeed("GameSys.DebugHUD.DebugMenu.Speed", 15.f, Debug::Range(3.f, 40.f));
Debug::Bool		gMenuRetainMenuPosition("GameSys.DebugHUD.DebugMenu.RetainSelectedMenu", true, Debug::Persist() );
Debug::Colour	gColour("GameSys.DebugHUD.DebugMenu.ColourSample", 0.5,1,0.4,1 );
Debug::Vector	gVector("GameSys.DebugHUD.DebugMenu.VectorSample", 0,0,0);


C_DebugHUDMenu * g_pDebugHUDMenu = NULL;

//////////////////////////////////////////////////////////////////////////
//	C_DebugHUDMenuSkin
//////////////////////////////////////////////////////////////////////////
void C_DebugHUDMenuSkin::Reset()
{
	m_Position				= Maths::HVec2( 512, 48.0f );
	m_TitleFontScale		= Maths::HVec2( 32.0f, 48.0f );
	m_NormalFontScale		= Maths::HVec2( 28.0f, 32.0f );

	m_SelctionColour		= Maths::RGBA(228,  224,    232,    0x20);
	m_MarginColour            = Maths::RGBA(27,  24,    96,    0xff);
																
	m_SelectedFontColour	= Maths::RGBA( 1.0f,1.0f,1.0f,1.0f );
	m_DeselectedFontColour	= Maths::RGBA( 0.0f,0.0f,0.0f,1.0f );
	m_TitleFontColour		= Maths::RGBA( 1.0f,1.0f,1.0f,1.0f );
		

	m_BorderSize			= Maths::HVec2( 8.0f, 6.0f );
	m_OutlineSize			= Maths::HVec2( 3.0f, 3.0f );
	m_MarginSize			= Maths::HVec2( 1.0f, 1.0f );

	m_MaxHeight				= 768.0f - 192.0f;
	
	m_FontName				= "dontship/fonts/arialdebug.font";

	//Default the debug layer
	m_pFrame = NULL;
}

//////////////////////////////////////////////////////////////////////////
//MenuHUDItem
//////////////////////////////////////////////////////////////////////////

void C_MenuHUDItem::Clear()
{
	m_pParent = NULL;
	m_pNext = NULL;
	m_pPrev = NULL;
	m_pFirstChild = NULL;
}

C_MenuHUDItem::~C_MenuHUDItem()
{
	// delete all child nodes
	C_MenuHUDItem* pCurrItem = m_pFirstChild;
	while(pCurrItem)
	{
		C_MenuHUDItem* pNextItem = pCurrItem->m_pNext;
		delete pCurrItem;
		pCurrItem = pNextItem;
	}
}


void C_MenuHUDItem::AddChild( C_MenuHUDItem* pChild )
{
	//Sort by name	
	C_MenuHUDItem* pMenuItem = m_pFirstChild;
	C_MenuHUDItem* pLastMenuItem = NULL;

	while (pMenuItem != NULL)
	{
		if (_stricmp(pMenuItem->Name().Chars(),pChild->Name().Chars()) > 0)
		{
			//Place Found
			break;
		}
		pLastMenuItem = pMenuItem;
		pMenuItem = pMenuItem->m_pNext;
	}
	
	//Add
	pChild->m_pParent = this;
	pChild->m_pNext = pMenuItem;
	pChild->m_pPrev = pLastMenuItem;

	if (pChild->m_pNext != NULL)
	{
		pChild->m_pNext->m_pPrev = pChild;
	}

	if (pChild->m_pPrev != NULL)
	{
		pChild->m_pPrev->m_pNext = pChild;

		//Second .. Last
	}
	else
	{
		//First
		m_pFirstChild = pChild;
	}
}

//////////////////////////////////////////////////////////////////////////
//DebugHUDMenu
//////////////////////////////////////////////////////////////////////////

C_DebugHUDMenu::C_DebugHUDMenu(I_Controller* pController, const C_DebugHUDMenuSkin PCREF debugHUDMenuSkin )
{
	m_Active = false;
	m_Keydown = 0;
	m_pController = pController;

	//Create root node
	m_pRootMenu = new C_MenuHUDItem( "Debug" );
	m_pCurrentMenu = m_pRootMenu;

	//Font renderers
	m_pSelectedFont = Render::Font::FontSystem()->GetFontRenderer(debugHUDMenuSkin.m_FontName);
	if (m_pSelectedFont)
	{
		m_pSelectedFont->SetScaleX(debugHUDMenuSkin.m_NormalFontScale.X());
		m_pSelectedFont->SetScaleY(debugHUDMenuSkin.m_NormalFontScale.Y());
		m_pSelectedFont->SetColourTop(debugHUDMenuSkin.m_SelectedFontColour);
		m_pSelectedFont->SetColourBottom(debugHUDMenuSkin.m_SelectedFontColour);
	}

	m_pDeselectedFont = Render::Font::FontSystem()->GetFontRenderer(debugHUDMenuSkin.m_FontName);
	if (m_pDeselectedFont)
	{
		m_pDeselectedFont->SetScaleX(debugHUDMenuSkin.m_NormalFontScale.X());
		m_pDeselectedFont->SetScaleY(debugHUDMenuSkin.m_NormalFontScale.Y());
		m_pDeselectedFont->SetColourTop(debugHUDMenuSkin.m_DeselectedFontColour);
		m_pDeselectedFont->SetColourBottom(debugHUDMenuSkin.m_DeselectedFontColour);
	}

	m_pTitleFont = Render::Font::FontSystem()->GetFontRenderer(debugHUDMenuSkin.m_FontName);
	if (m_pTitleFont)
	{
		m_pTitleFont->SetScaleX(debugHUDMenuSkin.m_TitleFontScale.X());
		m_pTitleFont->SetScaleY(debugHUDMenuSkin.m_TitleFontScale.Y());
		m_pTitleFont->SetColourTop(debugHUDMenuSkin.m_TitleFontColour);
		m_pTitleFont->SetColourBottom(debugHUDMenuSkin.m_TitleFontColour);
		m_pTitleFont->SetShadowEnable( true );
		m_pTitleFont->SetColourOutline(Maths::RGBA(0,0,25,255));
		m_pTitleFont->SetOutlineSize(0.35f);
	}
	
	m_MarginBlendMode = (debugHUDMenuSkin.m_MarginColour.A() < 255);
	m_HighlightBlendMode = (debugHUDMenuSkin.m_SelctionColour.A() < 255);

	Active(false);

	m_Position = debugHUDMenuSkin.m_Position;

	m_Skin = debugHUDMenuSkin;
	
	if (debugHUDMenuSkin.m_pFrame == NULL)
	{
		//Default frame
		m_pFrame = C_Frame::GetCommonFrame();
	}
	else
		m_pFrame = debugHUDMenuSkin.m_pFrame;

	UpdatePosition();

	//Add resume
	C_ResumeMenuItem* pResumeMenuItem = new C_ResumeMenuItem(this);
	m_pRootMenu->AddChild(pResumeMenuItem);

	g_pDebugHUDMenu = this;
}

C_DebugHUDMenu::~C_DebugHUDMenu()
{
	delete m_pSelectedFont;
	delete m_pDeselectedFont;
	delete m_pTitleFont;

	delete m_pRootMenu;
}

Debug::Bool	g_DebugDisableOnScreenDebugMenu( "GameSys.DisableOnScreenDebugMenu", false );

void C_DebugHUDMenu::Update(const Maths::C_TimeS& initialTimeStep)
{
	if( m_Active )
	{
		Maths::C_TimeS timeStep = initialTimeStep;
		if( timeStep > (1.0f / 15.0f) )
		{
			timeStep = Maths::C_TimeS(1.0f / 15.0f);
		}

		bool keydown = true;

		if( m_pController->Close())
		{
			Active( false );
		}
		else
		if( m_pController->Cancel()  )
		{
			Backwards();

			RefreshMenuSize( );
		}
		else
		if( m_pController->Confirm()  )
		{
			Forwards();

			RefreshMenuSize( );
		}
		else
		if( m_pController->MoveUp() )
		{
			Up();
		}
		else
		if( m_pController->MoveDown()  )
		{
			Down();
		}
		else
		if( m_pController->MoveLeft()  )
		{
			m_pCurrentMenu->Decrease( m_Keydown );

			RefreshMenuSize( );
		}
		else
		if( m_pController->MoveRight()  )
		{
			m_pCurrentMenu->Increase( m_Keydown );

			RefreshMenuSize( );
		}
		else
			keydown = false;

		if( keydown )
			m_Keydown++;
		else
			m_Keydown = 0;


		if( gMenuPositionX != U32(m_Position.X()) || gMenuPositionY != U32(m_Position.Y()) )
		{
			m_Position.X((F32)gMenuPositionX);
			m_Position.Y((F32)gMenuPositionY);
			UpdatePosition();
		}
		else
		{
			F32 deltaX, deltaY;
			if (m_pController->PositionMove(deltaX, deltaY))
			{
				m_Position = m_Position + Maths::HVec2(deltaX * gMenuSpeed , deltaY * gMenuSpeed ) ;
				UpdatePosition();
			}
		}

		
	}
	else
	if( !g_DebugDisableOnScreenDebugMenu && m_pController->Open() )
	{
		Active( true );
		m_Keydown = 0;
	}
}

void C_DebugHUDMenu::Render()
{
	if (!m_Active || !m_pTitleFont || !m_pSelectedFont || !m_pDeselectedFont)
		return;

	PERF_EVENT("Render DebugHUDMenu");

	// Title size
	F32 titleHeight;
	{
		Maths::HVec2 dimensions(m_pTitleFont->GetDimensions(0,"H"));
		titleHeight = dimensions.Y();
	}
	titleHeight += m_Skin.m_BorderSize.Y();

	// Item size
	F32 textMaxWidth = (m_ValuePos.X() - m_NamePos.X());
	F32 textHeight = 0.0f;
	U32 totalItems = 0;
	{
		Maths::HVec2 pos0( m_NamePos - m_TitlePos );
		for(C_MenuHUDItem *p = m_pCurrentMenu->m_pParent->m_pFirstChild; p; p = p->m_pNext)
		{
			Maths::HVec2 dimensions( m_pDeselectedFont->GetDimensions(0,p->Name()) );

			if(pos0.Y() + dimensions.Y() + m_Skin.m_OutlineSize.Y() > m_Skin.m_MaxHeight )
				break;

			pos0.Y( pos0.Y() + dimensions.Y() );
			totalItems++;
		}

		textHeight = pos0.Y();
	}

	// Count down from the start - makes the arrow start at the top of the list when it's at the start of the list
	const U32 listCentre = (totalItems / 2);
	C_MenuHUDItem *pStartItem = m_pCurrentMenu;
	U32 myCount;
	for(myCount = listCentre + (totalItems&1); pStartItem->m_pPrev && myCount; myCount--)
		pStartItem = pStartItem->m_pPrev;

	// Count back from the end - makes the arrow drop to the bottom of thet screen at the end of the list
	if(myCount==0)
	{
		U32 endCount;

		C_MenuHUDItem *pEndItem = m_pCurrentMenu;

		for(endCount = listCentre - 1; pEndItem->m_pNext && endCount; endCount--)
			pEndItem = pEndItem->m_pNext;

		while(endCount-- && pStartItem->m_pPrev ) 
			pStartItem = pStartItem->m_pPrev;
	}

	// Window size
	Maths::HVec2 textSize( textMaxWidth, textHeight );
	Maths::HVec2 windowSize( textSize + ((m_Skin.m_BorderSize + m_Skin.m_OutlineSize) * 2.0f));
	Maths::HVec2 windowPos( m_TitlePos.X() - (windowSize.X() * 0.5f), m_TitlePos.Y() );
	
	F32 typeWidth = m_ValueWidth + (m_Skin.m_BorderSize.X() * 3.0f);

	if (m_pFrame)
		m_pFrame->Draw(windowPos, windowSize);

	// Margins
	if(m_Skin.m_MarginSize.X() > 0.0f || m_Skin.m_MarginSize.Y() > 0.0f)
	{
		Render::Debug::Immediate()->SetBlendMode((m_MarginBlendMode)?DbUtil::E_BlendMode_Alpha:DbUtil::E_BlendMode_Solid);
		Render::Debug::Immediate()->Draw2dBar( Maths::HVec2(windowPos.X() + windowSize.X() - typeWidth, windowPos.Y() + titleHeight), Maths::HVec2( m_Skin.m_MarginSize.X(), windowSize.Y() - titleHeight), m_Skin.m_MarginColour);
		Render::Debug::Immediate()->Draw2dBar( Maths::HVec2(windowPos.X(), windowPos.Y() + titleHeight), Maths::HVec2( windowSize.X(), m_Skin.m_MarginSize.Y()), m_Skin.m_MarginColour);
		Render::Debug::Immediate()->SetBlendMode(DbUtil::E_BlendMode_Solid);
	}

	// Title
	if(m_pCurrentMenu->m_pParent)
	{
		Maths::RGBA backTitleColour;
		if (m_pCurrentMenu->m_pParent->BackColour(backTitleColour))
		{
			Render::Debug::Immediate()->Draw2dBar( windowPos, Maths::HVec2(windowSize.X(), titleHeight ), backTitleColour);

			Maths::RGBA oldTopColour = m_pTitleFont->GetColourTop();
			Maths::RGBA oldBottomColour = m_pTitleFont->GetColourBottom();
			m_pTitleFont->SetColourTop(Maths::RGBA(255 - backTitleColour.R(), 255 - backTitleColour.G(), 255 - backTitleColour.B(), 255));
			m_pTitleFont->SetColourBottom(Maths::RGBA(255 - backTitleColour.R(), 255 - backTitleColour.G(), 255 - backTitleColour.B(), 255));
			m_pTitleFont->Print(m_TitlePos, m_pCurrentMenu->m_pParent->Name(), Render::Font::E_Justification_Centre);
			m_pTitleFont->SetColourTop(oldTopColour);
			m_pTitleFont->SetColourBottom(oldBottomColour);
		}
		else
			m_pTitleFont->Print(m_TitlePos, m_pCurrentMenu->m_pParent->Name(), Render::Font::E_Justification_Centre);
	}

	// Items
	{
		Maths::HVec2 pos0( m_NamePos );
		Maths::HVec2 pos1( m_ValuePos );
		C_MenuHUDItem *p = pStartItem;
		for(U32 itemCount = 0; itemCount < totalItems && p; itemCount++)
		{
			Maths::HVec2						nameDimensions( m_pDeselectedFont->GetDimensions(0,p->Name()) );
			Render::Font::C_FontRenderer*		pFontRenderer;

			if( (itemCount== 0 &&  pStartItem != p->m_pParent->m_pFirstChild) || (itemCount == totalItems - 1 && p->m_pNext) )
				m_pSelectedFont->Print(pos0, "<more>...");
			else
			{
				if(p == m_pCurrentMenu)
				{
					Render::Debug::Immediate()->SetBlendMode((m_HighlightBlendMode)?DbUtil::E_BlendMode_Alpha:DbUtil::E_BlendMode_Solid); 
					Render::Debug::Immediate()->Draw2dBar( Maths::HVec2( windowPos.X(), pos0.Y() ), Maths::HVec2(windowSize.X(), nameDimensions.Y() ), m_Skin.m_SelctionColour);
					pFontRenderer = m_pSelectedFont;
					Render::Debug::Immediate()->SetBlendMode(DbUtil::E_BlendMode_Solid); 
				}
				else
					pFontRenderer = m_pDeselectedFont;						

				Maths::RGBA backColour;
				bool bBackColour = p->BackColour(backColour);
				if (bBackColour)
				{
					Render::Debug::Immediate()->Draw2dBar(  Maths::HVec2( pos1.X() - m_ValueWidth, pos1.Y()) , Maths::HVec2(m_ValueWidth, nameDimensions.Y()), backColour);
				}
				
				pFontRenderer->Print( pos0, p->Name());
				if (bBackColour)
				{	
					Maths::RGBA oldTopColour = pFontRenderer->GetColourTop();
					Maths::RGBA oldBottomColour = pFontRenderer->GetColourBottom();
					pFontRenderer->SetColourTop(Maths::RGBA(255 - backColour.R(), 255 - backColour.G(), 255 - backColour.B(), 255));
					pFontRenderer->SetColourBottom(Maths::RGBA(255 - backColour.R(), 255 - backColour.G(), 255 - backColour.B(), 255));
					pFontRenderer->Print( pos1, p->Value(), Render::Font::E_Justification_Right);
					pFontRenderer->SetColourTop(oldTopColour);
					pFontRenderer->SetColourBottom(oldBottomColour);
				}
				else
				pFontRenderer->Print( pos1, p->Value(), Render::Font::E_Justification_Right);
			}

			pos0.Y( pos0.Y() + nameDimensions.Y() );
			pos1.Y( pos1.Y() + nameDimensions.Y() );

			p = p->m_pNext;
		}
	}
}

const bool C_DebugHUDMenu::Active()
{
	return m_Active;
}

void C_DebugHUDMenu::Active( const bool on )
{
	if(m_Active == on)
		return;

	m_Active = on;

	if( m_Active )
	{
		RebuildDebugVars( );
		RefreshMenuSize( );
	}
}

void C_DebugHUDMenu::RebuildDebugVars()
{

	Util::C_String temp("");

	if( gMenuRetainMenuPosition )
	{
		// Build total name
		for(C_MenuHUDItem *p = m_pCurrentMenu; p && p->m_pParent; p = p->m_pParent )
			temp = p->Name() + "." + temp;
	}

	// Delete the old list
	for(C_MenuHUDItem *p = m_pRootMenu->m_pFirstChild; p; p = p->m_pNext)
		if(p->Name() == g_DebugVarsName)
		{
			if(p->m_pParent->m_pFirstChild == p)
				p->m_pParent->m_pFirstChild = p->m_pNext;

 			if(p->m_pPrev)
				p->m_pPrev->m_pNext = p->m_pNext;

			if(p->m_pNext)
				p->m_pNext->m_pPrev = p->m_pPrev;

			delete p;
			
			break;
		}

	// Create new DebugVars root
	C_MenuHUDItem *pVarsRoot = new C_MenuHUDItem(g_DebugVarsName);
	m_pRootMenu->AddChild( pVarsRoot);

	// Create a new list
	for(Debug::Iterator it; it; ++it)
	{
		Util::C_String temp( it->Name() );
		C_MenuHUDItem *pMenu( pVarsRoot );

		do
		{
			S32 i = FindSeparator(temp);
			if( i == -1 )
			{
				// Create a new item
				C_MenuHUDItem *pNewItem = new C_DebugVarMenuItem(temp, it );
				pMenu->AddChild( pNewItem );
				break;
			}
			else
			{
				Util::C_String menuName = temp( 0, i );
				temp = temp( i+1, temp.Length() );

				bool found = false;
				for(C_MenuHUDItem *p = pMenu->m_pFirstChild; p; p = p->m_pNext)
					if(_stricmp(p->Name().Chars(), menuName.Chars()) == 0)
					{
						if(!p->m_pFirstChild) // Menu exists but is a variable not sub menu type
							break;

						found = true;
						pMenu = p;
						break;
					}
				// Create a new sub-menu for this item
				if(!found)
				{
					C_MenuHUDItem *pNewItem = new C_MenuHUDItem(menuName);
					pMenu->AddChild( pNewItem );
					pMenu = pNewItem;
				}
			}
		} while(1);
	}

	// Reset the menu pointer
	m_pCurrentMenu = m_pRootMenu->m_pFirstChild;

	if( gMenuRetainMenuPosition )
	{
		// Try and retain the old position
		C_MenuHUDItem* pMenu = m_pRootMenu;
		while( temp.Length() )
		{
			S32 i = FindSeparator(temp);
			Util::C_String searchString( (i == -1 ) ? temp : temp( 0, i ));

			bool menuFound = false;

			for(C_MenuHUDItem *p = pMenu->m_pFirstChild; p; p = p->m_pNext)
				if(p->Name() == searchString)
				{
					menuFound = true;
					pMenu = p;
					m_pCurrentMenu = p;
					break;
				}

			if(i == -1 || !menuFound )
				break;

			temp = temp( i + 1, temp.Length() );
		}
	}
}

S32 C_DebugHUDMenu::FindSeparator( Util::C_String &string )
{
	S32 i = string.Search(".");
	if (i != -1)
		return i;

	return string.Search("|");
}

void C_DebugHUDMenu::CalcMenuExtentsRecursive( F32& maxName, F32& maxValue, C_MenuHUDItem *pItem )
{
	if(!m_pCurrentMenu->m_pParent)
		return;

	m_NameWidth = 120.0f;
	m_ValueWidth = 40.0f;

	CalcMenuExtents( m_NameWidth, m_ValueWidth, m_pCurrentMenu->m_pParent );

	for(C_MenuHUDItem *p = m_pCurrentMenu->m_pParent->m_pFirstChild; p; p = p->m_pNext)
		CalcMenuExtents( m_NameWidth, m_ValueWidth, p );

	F32 totalSize = m_NameWidth + m_ValueWidth + (m_Skin.m_BorderSize.X() * 4.0f);

	m_NamePos.X(  m_Position.X() - ( totalSize * 0.5f ));
	m_ValuePos.X( m_Position.X() + ( totalSize * 0.5f ));
}

void C_DebugHUDMenu::CalcMenuExtents( F32& maxName, F32& maxValue, C_MenuHUDItem *pItem )
{
	{
		Maths::HVec2 size( m_pSelectedFont->GetDimensions( 0, pItem->Name() ));
		if( maxName < size.X() )
			maxName = size.X();
	}
	{
		Maths::HVec2 size( m_pSelectedFont->GetDimensions( 0, pItem->Value() ));
		if( maxValue < size.X() )
			maxValue = size.X();
	}
}

void C_DebugHUDMenu::RefreshMenuSize()
{
	if(!m_pCurrentMenu->m_pParent)
		return;

	m_NameWidth = 120.0f;
	m_ValueWidth = 40.0f;

	CalcMenuExtents( m_NameWidth, m_ValueWidth, m_pCurrentMenu->m_pParent );

	for(C_MenuHUDItem *p = m_pCurrentMenu->m_pParent->m_pFirstChild; p; p = p->m_pNext)
		CalcMenuExtents( m_NameWidth, m_ValueWidth, p );

	F32 totalSize = m_NameWidth + m_ValueWidth + (m_Skin.m_BorderSize.X() * 4.0f);

	m_NamePos.X( m_Position.X() - ( totalSize * 0.5f ));
	m_ValuePos.X( m_Position.X() + ( totalSize * 0.5f ));
}

void C_DebugHUDMenu::UpdatePosition()
{
	if (!m_pTitleFont)
		return;

	if (m_Position.X() > 1024.f)
		m_Position.X(1024.f);

	if (m_Position.Y() > 768.f)
		m_Position.Y(768.f);

	if (m_Position.X() < 0.f)
		m_Position.X(0.f);

	if (m_Position.Y() < 0.f)
		m_Position.Y(0.f);

	gMenuPositionX = (S32)m_Position.X();
	gMenuPositionY = (S32)m_Position.Y();

	F32 totalSize = m_NameWidth + m_ValueWidth + (m_Skin.m_BorderSize.X() * 4.0f);

	m_NamePos.X(  m_Position.X() - ( totalSize * 0.5f ));
	m_ValuePos.X( m_Position.X() + ( totalSize * 0.5f ));

	m_TitlePos = Maths::HVec2( m_Position.X(), m_Position.Y() );
	Maths::HVec2 dimensions( m_pTitleFont->GetDimensions( 0, "H" ) );

	m_NamePos.Y( m_TitlePos.Y() + dimensions.Y() + m_Skin.m_BorderSize.Y() * 3.0f );
	m_ValuePos.Y( m_NamePos.Y() );
}

void C_DebugHUDMenu::Up()
{
	if (m_pCurrentMenu->m_pPrev)
		m_pCurrentMenu = m_pCurrentMenu->m_pPrev;
}

void C_DebugHUDMenu::Down()
{
	if (m_pCurrentMenu->m_pNext)
		m_pCurrentMenu = m_pCurrentMenu->m_pNext;
}

void C_DebugHUDMenu::Forwards()
{
	//Callback function
	if (m_pCurrentMenu->Forwards())
	{
		//Navigate to the children
		if (m_pCurrentMenu->m_pFirstChild)
		{
			m_pCurrentMenu = m_pCurrentMenu->m_pFirstChild;
		}
	}
}

void C_DebugHUDMenu::Backwards()
{
	if (m_pCurrentMenu->m_pParent)
	{
		if (m_pCurrentMenu->m_pParent->m_pParent)
		{
			m_pCurrentMenu = m_pCurrentMenu->m_pParent;
		}
	}
}

C_DebugHUDMenu* C_DebugHUDMenu::GetInstance()
{
	return g_pDebugHUDMenu;
}

bool C_DebugHUDMenu::RemoveItem( C_MenuHUDItem* pRemoveItem )
{
	//Check if it is the current node
	if (m_pCurrentMenu == pRemoveItem)
	{
		if (pRemoveItem->m_pPrev)
			m_pCurrentMenu = pRemoveItem->m_pPrev;
		else if (pRemoveItem->m_pNext)
			m_pCurrentMenu = pRemoveItem->m_pNext;
		else if (pRemoveItem->m_pParent)
			m_pCurrentMenu = pRemoveItem->m_pParent;
		else
		{
			assert(false); //Removing root??
		}
	}

	//Remove from Menu
	if (pRemoveItem->m_pNext || pRemoveItem->m_pPrev)
	{
		if (pRemoveItem->m_pNext)
		{
			pRemoveItem->m_pNext->m_pPrev = pRemoveItem->m_pPrev;
		}
		if (pRemoveItem->m_pPrev)
		{
			pRemoveItem->m_pPrev->m_pNext = pRemoveItem->m_pNext;
		}
		else
		{
			//First
			if (pRemoveItem->m_pParent)
			{
				pRemoveItem->m_pParent->m_pFirstChild = pRemoveItem->m_pNext;
			}
		}
	}
	else
	{
		return false;
	}

	return true;
}
//////////////////////////////////////////////////////////////////////////
//	C_ResumeMenuItem
//////////////////////////////////////////////////////////////////////////
bool C_ResumeMenuItem::Forwards()
{
	//Disable menu
	m_pDebugHUDMenu->Active( false );
	
	return false;
}

//////////////////////////////////////////////////////////////////////////
//	C_DebugVarMenuItem
//////////////////////////////////////////////////////////////////////////

C_DebugVarMenuItem::C_DebugVarMenuItem( const Util::C_String name, Debug::VarBase* pOption ) :
C_MenuHUDItem(name)
{
	m_pOption = pOption;	

	//Check if Colour
	if (m_pOption->Type() == Debug::kVtColour)
	{
		//Create helpers to change the colour
		C_DebugVarMenuColourHelperItem *pHelperR = new C_DebugVarMenuColourHelperItem("1 Red", (Debug::Colour*)m_pOption, 0);
		C_DebugVarMenuColourHelperItem *pHelperG = new C_DebugVarMenuColourHelperItem("2 Green", (Debug::Colour*)m_pOption, 1);
		C_DebugVarMenuColourHelperItem *pHelperB = new C_DebugVarMenuColourHelperItem("3 Blue", (Debug::Colour*)m_pOption, 2);
		C_DebugVarMenuColourHelperItem *pHelperA = new C_DebugVarMenuColourHelperItem("4 Alpha", (Debug::Colour*)m_pOption, 3);

		AddChild(pHelperR);
		AddChild(pHelperG);
		AddChild(pHelperB);
		AddChild(pHelperA);
	}
	else
	//Check if vector
	if (m_pOption->Type() == Debug::kVtVector3)
	{
		C_DebugVarMenuVectorHelperItem *pHelperX =  new C_DebugVarMenuVectorHelperItem("X", (Debug::Vector*)m_pOption, 0);
		C_DebugVarMenuVectorHelperItem *pHelperY =  new C_DebugVarMenuVectorHelperItem("Y", (Debug::Vector*)m_pOption, 1);
		C_DebugVarMenuVectorHelperItem *pHelperZ =  new C_DebugVarMenuVectorHelperItem("Z", (Debug::Vector*)m_pOption, 2);

		AddChild(pHelperX);
		AddChild(pHelperY);
		AddChild(pHelperZ);
	}
}
C_DebugVarMenuItem::~C_DebugVarMenuItem()
{

}

bool C_DebugVarMenuItem::Forwards()
{
	if (m_pOption->Type() == Debug::kVtFunction)
	{
		Debug::Function* var = (Debug::Function*)m_pOption;

		var->Call();
	}
	if (m_pOption->Type() == Debug::kVtColour)
	{
		//Nothing
		return true;
	}
	else
	if (m_pOption->Type() == Debug::kVtVector3)
	{
		//Nothing
		return true;
	}
	else
		Debug::SetToDefault(m_pOption);

	return false;
}

void C_DebugVarMenuItem::Increase(const U32 keydown)
{
	Debug::Adjust(m_pOption, 1 + keydown/10);
}

void C_DebugVarMenuItem::Decrease(const U32 keydown)
{
	Debug::Adjust(m_pOption, -1 - keydown/10);
}

Util::C_String C_DebugVarMenuItem::Value()
{
	char buf[256];

	switch(m_pOption->Type())
	{
		case Debug::kVtBool:
		{
			Debug::Bool* var = (Debug::Bool*)m_pOption;
			_snprintf(buf, sizeof(buf), "%s", var->Value() ? "true" : "false");
		}
		break;

		case Debug::kVtInt:
		{
			Debug::Int* var = (Debug::Int*)m_pOption;
			_snprintf(buf, sizeof(buf), "%i", var->Value());
		}
		break;

		case Debug::kVtFloat:
		{
			Debug::Float* var = (Debug::Float*)m_pOption;
			_snprintf(buf, sizeof(buf), "%f", var->Value());
		}
		break;

		case Debug::kVtEnum:
		{
			Debug::EnumBase* var = (Debug::EnumBase*)m_pOption;
			_snprintf(buf, sizeof(buf), "%s", var->ValueAsName());
		}
		break;

		case Debug::kVtString:
		{
			Debug::String* var = (Debug::String*)m_pOption;
			_snprintf(buf, sizeof(buf), "%s", var->Value());
		}
		break;

		case Debug::kVtFunction:
		{
			Debug::Function* var = (Debug::Function*)m_pOption;
			_snprintf(buf, sizeof(buf), "(call)");
		}
		break;

		case Debug::kVtVector3:
		{
			Debug::Vector* var = (Debug::Vector*)m_pOption;
			_snprintf(buf, sizeof(buf), "%.2f %.2f %.2f", var->Value().vx, var->Value().vy, var->Value().vz);
		}
		break;

		case Debug::kVtColour:
		{
			Debug::Colour* var = (Debug::Colour*)m_pOption;
			_snprintf(buf, sizeof(buf), "%.2f %.2f %.2f %.2f", var->Value().r, var->Value().g, var->Value().b, var->Value().a);
		}
		break;

		default:
			_snprintf(buf, sizeof(buf), "Unknown");
		break;
	}

	return buf;
}

#define max(a,b) (a>b)?a:b
#define min(a,b) (a<b)?a:b

bool C_DebugVarMenuItem::BackColour( Maths::RGBA& backColour )
{
	if (m_pOption->Type() == Debug::kVtColour)
	{
		Debug::Colour* var = (Debug::Colour*)m_pOption;
		backColour = Maths::RGBA(min(S32(var->Value().r * 255),255), min(S32(var->Value().g * 255),255), min(S32(var->Value().b * 255),255), min(S32(var->Value().a * 255),255));
		return true;
	}
	else
		return false;
}

//////////////////////////////////////////////////////////////////////////
//C_DebugVarMenuColourHelperItem
//////////////////////////////////////////////////////////////////////////

C_DebugVarMenuColourHelperItem::C_DebugVarMenuColourHelperItem( const Util::C_String name, Debug::Colour* pColourOption, const U32 channel ): C_MenuHUDItem(name)
{
	m_pColourOption = pColourOption;
	m_Channel = channel;
}

void C_DebugVarMenuColourHelperItem::Increase(const U32 keydown)
{
	switch (m_Channel)
	{
	case 0:
		m_pColourOption->SetValue(m_pColourOption->Value().r + 1.f/256.f, m_pColourOption->Value().g, m_pColourOption->Value().b, m_pColourOption->Value().a);
		break;
	case 1:
		m_pColourOption->SetValue(m_pColourOption->Value().r, m_pColourOption->Value().g + 1.f/256.f, m_pColourOption->Value().b, m_pColourOption->Value().a);
		break;
	case 2:
		m_pColourOption->SetValue(m_pColourOption->Value().r, m_pColourOption->Value().g, m_pColourOption->Value().b + 1.f/256.f, m_pColourOption->Value().a);
		break;
	case 3:
		m_pColourOption->SetValue(m_pColourOption->Value().r, m_pColourOption->Value().g, m_pColourOption->Value().b, m_pColourOption->Value().a + 1.f/256.f);
		break;
	}
}

void C_DebugVarMenuColourHelperItem::Decrease(const U32 keydown)
{
	switch (m_Channel)
	{
	case 0:
		m_pColourOption->SetValue(max(m_pColourOption->Value().r - 1.f/256.f, 0.f), m_pColourOption->Value().g, m_pColourOption->Value().b, m_pColourOption->Value().a);
		break;
	case 1:
		m_pColourOption->SetValue(m_pColourOption->Value().r, max(m_pColourOption->Value().g - 1.f/256.f, 0.f), m_pColourOption->Value().b, m_pColourOption->Value().a);
		break;
	case 2:
		m_pColourOption->SetValue(m_pColourOption->Value().r, m_pColourOption->Value().g, max(m_pColourOption->Value().b - 1.f/256.f, 0.f), m_pColourOption->Value().a);
		break;
	case 3:
		m_pColourOption->SetValue(m_pColourOption->Value().r, m_pColourOption->Value().g, m_pColourOption->Value().b, max(m_pColourOption->Value().a - 1.f/256.f, 0.f));
		break;
	}
}

Util::C_String C_DebugVarMenuColourHelperItem::Value()
{
	char buf[256];
	buf[0] = 0;

	switch (m_Channel)
	{
	case 0:
		_snprintf(buf, sizeof(buf), "%.2f", m_pColourOption->Value().r);
		break;
	case 1:
		_snprintf(buf, sizeof(buf), "%.2f", m_pColourOption->Value().g);
		break;
	case 2:
		_snprintf(buf, sizeof(buf), "%.2f", m_pColourOption->Value().b);
		break;
	case 3:
		_snprintf(buf, sizeof(buf), "%.2f", m_pColourOption->Value().a);
		break;
	}

	return buf;
}

bool C_DebugVarMenuColourHelperItem::BackColour( Maths::RGBA& backColour )
{
	switch (m_Channel)
	{
	case 0:
		backColour = Maths::RGBA(min(S32(m_pColourOption->Value().r * 255),255), 0, 0 , 255);
		break;
	case 1:
		backColour = Maths::RGBA(0, min(S32(m_pColourOption->Value().g * 255),255), 0, 255);
		break;
	case 2:
		backColour = Maths::RGBA(0, 0, min(S32(m_pColourOption->Value().b * 255),255), 255);
		break;
	case 3:
		backColour = Maths::RGBA(min(S32(m_pColourOption->Value().a * 255),255), min(S32(m_pColourOption->Value().a * 255),255), min(S32(m_pColourOption->Value().a * 255),255) , 255);
		break;
	}
	return true;
}

bool C_DebugVarMenuColourHelperItem::Forwards()
{
	switch (m_Channel)
	{
	case 0:
		m_pColourOption->SetValue(m_pColourOption->Default().r,m_pColourOption->Value().g, m_pColourOption->Value().b, m_pColourOption->Value().a);
		break;
	case 1:
		m_pColourOption->SetValue(m_pColourOption->Value().r,m_pColourOption->Default().g, m_pColourOption->Value().b, m_pColourOption->Value().a);
		break;
	case 2:
		m_pColourOption->SetValue(m_pColourOption->Value().r,m_pColourOption->Value().g, m_pColourOption->Default().b, m_pColourOption->Value().a);
		break;
	case 3:
		m_pColourOption->SetValue(m_pColourOption->Value().r,m_pColourOption->Value().g, m_pColourOption->Value().b, m_pColourOption->Default().a);
		break;
	}

	return false;
}
//////////////////////////////////////////////////////////////////////////
//C_DebugVarMenuVectorHelperItem
//////////////////////////////////////////////////////////////////////////

C_DebugVarMenuVectorHelperItem::C_DebugVarMenuVectorHelperItem( const Util::C_String name, Debug::Vector* pVectorOption, const U32 index ) : C_MenuHUDItem(name)
{
	m_pVectorOption = pVectorOption;
	m_Index = index;
}

void C_DebugVarMenuVectorHelperItem::Increase(const U32 keydown)
{
	switch (m_Index)
	{
	case 0:
		m_pVectorOption->SetValue(m_pVectorOption->Value().vx + 0.01f * (1 + keydown), m_pVectorOption->Value().vy, m_pVectorOption->Value().vz);
		break;
	case 1:
		m_pVectorOption->SetValue(m_pVectorOption->Value().vx, m_pVectorOption->Value().vy + 0.01f * (1 + keydown), m_pVectorOption->Value().vz);
		break;
	case 2:
		m_pVectorOption->SetValue(m_pVectorOption->Value().vx, m_pVectorOption->Value().vy, m_pVectorOption->Value().vz + 0.01f * (1 + keydown));
		break;	
	}
}

void C_DebugVarMenuVectorHelperItem::Decrease(const U32 keydown)
{
	switch (m_Index)
	{
	case 0:
		m_pVectorOption->SetValue(m_pVectorOption->Value().vx - 0.01f * (1 + keydown), m_pVectorOption->Value().vy, m_pVectorOption->Value().vz);
		break;
	case 1:
		m_pVectorOption->SetValue(m_pVectorOption->Value().vx, m_pVectorOption->Value().vy - 0.01f * (1 + keydown), m_pVectorOption->Value().vz);
		break;
	case 2:
		m_pVectorOption->SetValue(m_pVectorOption->Value().vx, m_pVectorOption->Value().vy, m_pVectorOption->Value().vz - 0.01f * (1 + keydown));
		break;	
	}
}

Util::C_String C_DebugVarMenuVectorHelperItem::Value()
{
	char buf[256];
	buf[0] = 0;

	switch (m_Index)
	{
	case 0:
		_snprintf(buf, sizeof(buf), "%f", m_pVectorOption->Value().vx);
		break;
	case 1:
		_snprintf(buf, sizeof(buf), "%f", m_pVectorOption->Value().vy);
		break;
	case 2:
		_snprintf(buf, sizeof(buf), "%f", m_pVectorOption->Value().vz);
		break;	
	}

	return buf;
}

bool C_DebugVarMenuVectorHelperItem::Forwards()
{
	//Default value
	switch (m_Index)
	{
	case 0:
		m_pVectorOption->SetValue(m_pVectorOption->Default().vx, m_pVectorOption->Value().vy, m_pVectorOption->Value().vz);
		break;
	case 1:
		m_pVectorOption->SetValue(m_pVectorOption->Default().vx, m_pVectorOption->Default().vy, m_pVectorOption->Value().vz);
		break;
	case 2:
		m_pVectorOption->SetValue(m_pVectorOption->Value().vx, m_pVectorOption->Value().vy, m_pVectorOption->Default().vz);
		break;	
	}
	return false;
}
}


