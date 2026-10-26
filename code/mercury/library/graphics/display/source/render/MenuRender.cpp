#include "stdafx.h"
#include "Display/Render/MenuRender.h"
#include "Display/Render/BasicRender.h"
#include "Display/Device/Device.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF
#endif

namespace Display
{

C_MenuItem::C_MenuItem() :
	m_Name("")
{
	Clear();
}

C_MenuItem::C_MenuItem(const Util::C_String& name) :
	m_Name(name)
{
	Clear();
}

C_MenuItem::~C_MenuItem()
{
	m_pLastChild = 0;
	while (m_pFirstChild)
	{
		C_MenuItem * tmp = m_pFirstChild->m_pNext;
		delete m_pFirstChild;
		m_pFirstChild = tmp;
	}

	if (m_pNext) m_pNext->m_pPrev = m_pPrev;
	if (m_pPrev) m_pPrev->m_pNext = m_pNext;
}

void C_MenuItem::Clear()
{
	m_pParent = m_pPrev = m_pNext = m_pFirstChild = m_pLastChild = NULL;
}

Util::C_String C_MenuItem::GetValue()
{
	return m_pFirstChild ? Util::C_String("=>") : Util::C_String("");
}

void C_MenuItem::AddChild(C_MenuItem* pChild, const bool insertAtEnd)
{
	assert(pChild);
	pChild->m_pParent = this;

	if(insertAtEnd)
	{
		pChild->m_pNext = NULL;
		
		if(!m_pFirstChild)
		{
			m_pFirstChild = pChild;
		}

		if(!m_pLastChild)
		{
			pChild->m_pPrev = NULL;
		}
		else
		{
			pChild->m_pPrev = m_pLastChild;
			m_pLastChild->m_pNext = pChild;
		}

		m_pLastChild = pChild;
	}
	else
	{
		pChild->m_pPrev = NULL;

		if(!m_pLastChild)
		{
			m_pLastChild = pChild;
		}

		if(!m_pFirstChild)
		{
			pChild->m_pNext = NULL;
		}
		else
		{
			pChild->m_pNext = m_pFirstChild;
			m_pFirstChild->m_pPrev = pChild;
		}

		m_pFirstChild = pChild;
	}
}

void C_MenuItem::Up()
{
	if(MenuRenderExists())
	{
		MenuRender()->Up();
	}
}

void C_MenuItem::Dn()
{
	if(MenuRenderExists())
	{
		MenuRender()->Dn();
	}
}

void C_MenuItem::Lt()
{
	if(MenuRenderExists())
	{
		MenuRender()->Lt();
	}
}

void C_MenuItem::Rt()
{
	if(MenuRenderExists())
	{
		MenuRender()->Rt();
	}
}

void C_MenuItem::Inc()
{
}

void C_MenuItem::Dec()
{
}

C_MenuItemOption::C_MenuItemOption(Debug::VarBase* pOption) :
	m_pOption(pOption)
{
}

C_MenuItemOption::~C_MenuItemOption()
{
}

Util::C_String C_MenuItemOption::GetValue()
{
	char value[256];

	assert(m_pOption);
	switch(m_pOption->Type())
	{
	case Debug::kVtBool:
	{
		Debug::Bool* pVar = (Debug::Bool*)m_pOption;
		sprintf_s(value, sizeof(value), pVar->Value() ? "true" : "false");
	}
	break;

	case Debug::kVtInt:
	{
		Debug::Int* pVar = (Debug::Int*)m_pOption;
		sprintf_s(value, sizeof(value), "%i", pVar->Value());
	}
	break;

	case Debug::kVtFloat:
	{
		Debug::Float* pVar = (Debug::Float*)m_pOption;
		sprintf_s(value, sizeof(value), "%f", pVar->Value());
	}
	break;

	case Debug::kVtEnum:
	{
		Debug::EnumBase* pVar = (Debug::EnumBase*)m_pOption;
		sprintf_s(value, sizeof(value), "%s", pVar->ValueAsName());
	}
	break;

	case Debug::kVtString:
	{
		Debug::String* pVar = (Debug::String*)m_pOption;
		sprintf_s(value, sizeof(value), "%s", pVar->Value());
	}
	break;

	case Debug::kVtFunction:
	{
		Debug::Function* pVar = (Debug::Function*)m_pOption;
		sprintf_s(value, sizeof(value), "(call)");
	}
	break;

	case Debug::kVtVector3:
	{
		Debug::Vector* pVar = (Debug::Vector*)m_pOption;
		sprintf_s(value, sizeof(value), "%.2f %.2f %.2f", pVar->Value().vx, pVar->Value().vy, pVar->Value().vz);
	}
	break;

	case Debug::kVtColour:
	{
		Debug::Colour* pVar = (Debug::Colour*)m_pOption;
		sprintf_s(value, sizeof(value), "%.2f %.2f %.2f %.2f", pVar->Value().r, pVar->Value().g, pVar->Value().b, pVar->Value().a);
	}
	break;

	default:
	{
		sprintf_s(value, sizeof(value), "Unknown");
	};
	break;
	}

	return value;
}

void C_MenuItemOption::Update(const S32 delta)
{
	Debug::Adjust(m_pOption, delta);
}

void C_MenuItemOption::Inc()
{
	Update(1);
}

void C_MenuItemOption::Dec()
{
	Update(-1);
}

C_MenuRender::C_MenuRender() :
	m_Show(false),
	m_X(512.f),
	m_Y(192.f),
	m_W(1.f),
	m_H(1.f)
{
	m_pCurrent = NULL;
	m_pRootMenu = new C_MenuItem();
	m_pRootMenu->SetName("Debug");

	Build();

	assert(g_pMenuRender == NULL);
	g_pMenuRender = this;
}

C_MenuRender::~C_MenuRender()
{
	SAFE_DELETE_PTR(m_pRootMenu);

	assert(g_pMenuRender == this);
	g_pMenuRender = NULL; 
}

C_MenuItem* C_MenuRender::GetCurrItem()
{
	return m_pCurrent;
}

void C_MenuRender::SetCurrItem(C_MenuItem* pCurrItem)
{
	m_pCurrent = pCurrItem;
}

void C_MenuRender::DrawRect(const F32 x, const F32 y, const F32 w, const F32 h, const Maths::RGBA colour)
{
	F32 x1 = (x * m_W) + m_X;
	F32 y1 = (y * m_H) + m_Y;
	F32 w1 = w * m_W;
	F32 h1 = h * m_H;
	BasicRender()->DrawColoured2DRect(x1, y1, w1, h1, colour);
}

void C_MenuRender::DrawFont(const Util::C_String& text, const F32 x, const F32 y, const F32 w, const F32 h, const Maths::RGBA colour, const E_FontRenderAlign align)
{
	F32 x1 = (x * m_W) + m_X;
	F32 y1 = (y * m_H) + m_Y;
	F32 w1 = w * m_W;
	F32 h1 = h * m_H;
	FontRender()->Draw(text.Chars(), x1, y1, w1, h1, colour, align);
}

void C_MenuRender::Draw()
{
	if(!m_Show || !m_pCurrent || !m_pCurrent->m_pParent)
		return;

	const F32 font_w = 20.f;
	const F32 font_h = 20.f;
	const F32 font_w_spacing = 5.f;
	const F32 font_h_spacing = 5.f;
	const F32 border = 2.f;
	const F32 margin = 3.f;
	const F32 title_h = 40.f;

	struct C_Rect
	{
		C_Rect(const F32 X, const F32 Y, const F32 W, const F32 H) : x(X), y(Y), w(W), h(H)
		{
		}

		F32 x, y, w, h;
	};

	C_MenuItem* pRoot = m_pCurrent->m_pParent;

	F32 col1 = 0.f;
	F32 col2 = 0.f;
	F32 row = title_h;
	{
		C_MenuItem* pTemp = pRoot->m_pFirstChild;
		while(pTemp)
		{
			const F32 spacing1 = FontRender()->GetSpacing(pTemp->GetName().Chars(), font_w);
			if(spacing1 > col1)
				col1 = spacing1;

			const F32 spacing2 = FontRender()->GetSpacing(pTemp->GetValue().Chars(), font_w);
			if(spacing2 > col2)
				col2 = spacing2;

			row += font_h + font_h_spacing;
			
			pTemp = pTemp->m_pNext;
		}
	}
	col1 += 30.f;
	if(col1 < 200.f)
		col1 = 200.f;
	col2 += 30.f;
	if(col2 < 75.f)
		col2 = 75.f;

	C_Rect title(0.f, 0.f, col1 + col2, title_h);
	C_Rect items(0.f, 0.f, col1, row);
	C_Rect datas(items.w, title.h, col2, row);

	C_Rect total(0.f, 0.f, col1 + col2, title.h + datas.h);

	const Maths::RGBA backgroundColour(128, 0, 0, 160);
	const Maths::RGBA foregroundColour(174, 114, 55, 160);
	const Maths::RGBA marginColour(188, 158, 122, 255);
	const Maths::RGBA titleColour(255, 255, 255, 255);
	const Maths::RGBA selectedColour(0xe0, 0xe0, 0xe0, 0xff);
	const Maths::RGBA unselectedColour(0x10, 0x10, 0x10, 0xff);

	F32 x = 0.f;
	F32 y = 0.f;
	F32 w = 0.f;
	F32 h = 0.f;

	// background
	{
		x = total.x;
		y = total.y;
		w = total.w;
		h = total.h;
		DrawRect(x, y, w, h, backgroundColour);

		x = x + border;
		y = y + border;
		w = w - border * 2.f;
		h = h - border * 2.f;
		DrawRect(x, y, w, h, foregroundColour);
	}

	// margins
	{
		x = total.x + border;
		y = total.y + title.h - (margin / 2.f);
		w = total.w - border * 2.f;
		h = margin;
		DrawRect(x, y, w, h, marginColour);

		x = datas.x - (margin / 2.f);
		y = datas.y;
		w = margin;
		h = datas.h - border;
		DrawRect(x, y, w, h, marginColour);
	}

	// title
	if(pRoot)
	{
		x = (title.x + title.w) / 2.f;
		y = (title.y + title.h) / 2.f;
		w = font_w * 1.25f;
		h = font_h * 1.25f;
		DrawFont(pRoot->GetName(), x, y, w, h, titleColour, E_FontRenderAlign_Ct);
	}

	// items
	if(pRoot)
	{
		x = items.x + font_w_spacing;
		y = items.y + title.y + title.h + font_h + font_h_spacing;
		w = font_w;
		h = font_h;
		
		C_MenuItem* pTemp = pRoot->m_pFirstChild;
		while(pTemp)
		{
			DrawFont(pTemp->GetName(), x, y, w, h, pTemp == m_pCurrent ? selectedColour : unselectedColour, E_FontRenderAlign_Lt);
			y += (font_h * 1.25f);
			
			pTemp = pTemp->m_pNext;
		}
	}

	// datas
	if(pRoot)
	{
		x = items.x + items.w + datas.w - font_w_spacing;
		y = items.y + title.y + title.h + font_h + font_h_spacing;
		w = font_w;
		h = font_h;
		
		C_MenuItem* pTemp = pRoot->m_pFirstChild;
		while(pTemp)
		{
			DrawFont(pTemp->GetValue(), x, y, w, h, pTemp == m_pCurrent ? selectedColour : unselectedColour, E_FontRenderAlign_Rt);
			y += (font_h * 1.25f);
			
			pTemp = pTemp->m_pNext;
		}
	}
}

void C_MenuRender::Build()
{
	C_MenuItem* pRootVars = m_pRootMenu;

	for(Debug::Iterator it; it; ++it)
	{
		Util::C_String temp(it->Name());

		C_MenuItem* pMenu(pRootVars);

		do
		{
			S32 i = FindSeperator(temp);
			if(i == -1)
			{
				C_MenuItem* pNewMenu = new C_MenuItemOption(it);
				pNewMenu->SetName(temp);
				pMenu->AddChild(pNewMenu);
				break;
			}
			else
			{
				Util::C_String menuName = temp(0, i);
				temp = temp(i + 1, temp.Length());

				bool found = false;
				for(C_MenuItem* pPtr = pMenu->m_pFirstChild; pPtr; pPtr = pPtr->m_pNext)
				{
					if(pPtr->GetName() == menuName)
					{
						if(!pPtr->m_pFirstChild)
						{
							break; // menu exists but is a variablre nopt a sub menu type
						}

						found = true;
						pMenu = pPtr;
						break;
					}
				}

				if(!found)
				{
					C_MenuItem* pNewMenu = new C_MenuItem();
					pNewMenu->SetName(menuName);
					pMenu->AddChild(pNewMenu);
					pMenu = pNewMenu;
				}
			}
		} while(1);
	}

	m_pCurrent = m_pRootMenu->m_pFirstChild;
}

S32 C_MenuRender::FindSeperator(Util::C_String temp)
{
	S32 i = temp.Search(".");
	return i != -1 ? i : temp.Search(".");
}

void C_MenuRender::Up()
{
	if(m_pCurrent && m_pCurrent->m_pPrev)
	{
		m_pCurrent = m_pCurrent->m_pPrev;
	}
}

void C_MenuRender::Dn()
{
	if(m_pCurrent && m_pCurrent->m_pNext)
	{
		m_pCurrent = m_pCurrent->m_pNext;
	}
}

void C_MenuRender::Lt()
{
	if(m_pCurrent && m_pCurrent->m_pParent && m_pCurrent->m_pParent->m_pParent)
	{
		m_pCurrent = m_pCurrent->m_pParent;
	}
}

void C_MenuRender::Rt()
{
	if(m_pCurrent && m_pCurrent->m_pFirstChild)
	{
		m_pCurrent = m_pCurrent->m_pFirstChild;
	}
}

C_MenuRender* g_pMenuRender = NULL;

void MenuRender(C_MenuRender* pMenuRender)
{
	g_pMenuRender = pMenuRender;
}

}; // namespace Display
