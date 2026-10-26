#ifndef DISPLAY_RENDER_MENU_RENDER_H
#define DISPLAY_RENDER_MENU_RENDER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Render/FontRender.h"

namespace Display
{

class C_MenuItem
{
public:

	C_MenuItem();
	C_MenuItem(const Util::C_String& name);
	virtual ~C_MenuItem();

	virtual Util::C_String GetValue();

	const Util::C_String& GetName();
	void SetName(const Util::C_String& name);

	void AddChild(C_MenuItem* pChild, const bool insertAtEnd = false);

	virtual void Up();
	virtual void Dn();
	virtual void Lt();
	virtual void Rt();
	virtual void Inc();
	virtual void Dec();

private:

	void Clear();

public:

	C_MenuItem*		m_pParent;
	C_MenuItem*		m_pPrev;
	C_MenuItem*		m_pNext;
	C_MenuItem*		m_pFirstChild;
	C_MenuItem*		m_pLastChild;

private:

	Util::C_String m_Name;
};

class C_MenuItemOption : public C_MenuItem
{
public:

	C_MenuItemOption(Debug::VarBase* pOption);
	~C_MenuItemOption();

	Util::C_String GetValue();

	void Inc();
	void Dec();

private:

	void Update(const S32 delta);

private:

	Debug::VarBase* m_pOption;
};

class C_MenuRender
{
public:

	C_MenuRender();
	~C_MenuRender();

	void IncX(const F32 delta);
	void IncY(const F32 delta);
	void IncW(const F32 delta);
	void IncH(const F32 delta);
	
	bool GetShow();
	void SetShow(const bool show);

	C_MenuItem* GetCurrItem();
	void SetCurrItem(C_MenuItem* pCurr);

	void Draw();
	void Build();

	void Up();
	void Dn();
	void Lt();
	void Rt();

private:

	void DrawRect(const F32 x, const F32 y, const F32 w, const F32 h, const Maths::RGBA colour);
	void DrawFont(const Util::C_String& text, const F32 x, const F32 y, const F32 w, const F32 h, const Maths::RGBA colour, const E_FontRenderAlign align);

	S32 FindSeperator(Util::C_String temp);

public:

	C_MenuItem* m_pCurrent;

private:

	bool			m_Show;
	F32				m_X;
	F32				m_Y;
	F32				m_W;
	F32				m_H;
	C_MenuItem*		m_pRootMenu;
};

extern C_MenuRender* g_pMenuRender;
void MenuRender(C_MenuRender* pMenuRender);
C_MenuRender* MenuRender();
const bool MenuRenderExists();

}; // namespace Display

#include "Display/Render/MenuRender.inl"

#endif // DISPLAY_RENDER_MENU_RENDER_H
