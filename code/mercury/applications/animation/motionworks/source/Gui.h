#ifndef GUI_H
#define GUI_H

#include <btl/Containers/OpenHashMap.h>

class C_GuiRender;

class C_Rect
{
public:
	C_Rect(): x(0.f), y(0.f), w(0.f), h(0.f) {}
	C_Rect(F32 inX, F32 inY, F32 inW, F32 inH): x(inX), y(inY), w(inW), h(inH) {}
	
	bool Inside(F32 px, F32 py) const
	{
		if (px < x || py < y || px >= (x+w) || py >= (y+h)) return false;
		return true;
	}
	
	void Enclose(const C_Rect& other)
	{
		F32 x1 = Maths::Min(x, other.x);
		F32 y1 = Maths::Min(y, other.y);
		F32 x2 = Maths::Max(x + w, other.x + other.w);
		F32 y2 = Maths::Max(y + h, other.y + other.h);
		
		x = x1;
		y = y1;
		w = x2 - x1;
		h = y2 - y1;
	}
	
	F32	x, y, w, h;
};

class GroupFlags
{
public:
	enum Flags
	{
		LayoutVertical		= 1 << 0,
		LayoutHorizontal	= 1 << 1,
		Indent				= 1 << 2,
		AlignLeft			= 1 << 3,
		AlignRight			= 1 << 4,
		AlignTop			= 1 << 5,
		AlignBottom			= 1 << 6
	};
};

class C_Camera;

class C_Gui
{
public:
	C_Gui(C_GuiRender& r);

	void SetFixedWidth(F32 fixedWidth);
	void SetAutoWidth();
	void SetSpacing(F32 spacing);
	void SetIndentSize(F32 s);
	void SetWindowSize(F32 w, F32 h);

	void Begin();
	void End();
	
	void BeginGroup(U32 flags, F32 x = 0.f, F32 y = 0.f, F32 w = 0.f, F32 h = 0.f);
	void EndGroup();
	
	void BeginChildren();
	void EndChildren();

	bool Button(U32 id, const CHAR8* caption);
	
	bool CheckBox(U32 id, const CHAR8* caption, bool enabled);
	
	void Label(const CHAR8* text);
	
	bool Camera(U32 id, F32 x, F32 y, F32 w, F32 h, C_Camera& camera);
	
	bool TreeNode(U32 id, const CHAR8* caption, bool openByDefault);
	
	void ColourSwatch(F32 w, F32 h, const Maths::RGBA& colour);
	
	U32 ComboBox(U32 id, const CHAR8** options, U32 numOptions, U32 selected);
	
	F32 HorizontalSlider(U32 id, F32 width, F32 minValue, F32 maxValue, F32 currentValue);

public:
	class C_Mouse
	{
	public:
		C_Mouse(): x(0.f), y(0.f), dragX(0.f), dragY(0.f), prevX(0.f), prevY(0.f) {}
		
		F32	x, y;
		F32 dragX, dragY;
		F32 prevX, prevY;
		
		class C_Buttons
		{
		public:
			C_Buttons(): left(false), middle(false), right(false), up(false), down(false) {}
			bool left;
			bool middle;
			bool right;
			bool up;
			bool down;
		};
		
		C_Buttons	buttons;
	};
	
	C_Mouse		mouse;


private:

	bool DoButton(U32 id, const C_Rect& r);

	bool IsActive(U32 id) const;
	bool IsHot(U32 id) const;
	void SetHot(U32 id);
	void SetActive(U32 id);
	void ClearActive();
	
	C_Rect PlaceRect(F32 w, F32 h);
	
	struct Group
	{
		C_Rect		rect;
		U32			flags;
	};
	static const U32 kMaxGroups = 10;
	Group			m_groupStack[kMaxGroups];
	U32				m_groupIndex;

	C_GuiRender&	m_render;
	U32				m_active;
	U32				m_hot;
	U32				m_hotThisFrame;
	bool			m_autoWidth;
	F32				m_fixedWidth;
	F32				m_spacing;
	F32				m_indentSize;
	C_Rect			m_window;
	
	typedef Btl::OpenHashMap<U32, U32> IdToStateMap;
	
	
	IdToStateMap		m_controlStates;
};

namespace Render
{
namespace Font
{
	class C_FontRenderer;
}
}

class C_GuiRender
{
public:
	C_GuiRender();
	
	void MeasureText(const CHAR8* str, F32& outW, F32& outH);
	void MeasureButton(const CHAR8* str, F32& outW, F32& outH);
	void MeasureLabel(const CHAR8* str, F32& outW, F32& outH);
	void MeasureCheckBox(const CHAR8* str, F32& outW, F32& outH);
	void MeasureTreeNode(const CHAR8* str, F32& outW, F32& outH);
	//void MeasureComboBox(const CHAR8** options, U32 numOptions, F32& outW, F32& outH);
	
	void DrawLabel(const C_Rect& r, const CHAR8* str, bool hot);
	void DrawButton(const C_Rect& r, const CHAR8* str, bool active, bool hot);
	void DrawCheckBox(const C_Rect& r, const CHAR8* str, bool enabled, bool active, bool hot);
	void DrawTreeNode(const C_Rect& r, const CHAR8* str, bool open, bool active, bool hot);
	void DrawColourSwatch(const C_Rect& r, const Maths::RGBA& colour);
	void DrawFrame(const C_Rect& r);
	void DrawHorizontalSlider(const C_Rect& r, F32 fraction, bool hot, bool active);

private:
	void SetTextColour(bool hot, bool active);

	Maths::RGBA		m_hotTextColour;
	Maths::RGBA		m_activeTextColour;
	Maths::RGBA		m_textColour;
		
	Render::Font::C_FontRenderer*	m_font;
};





#endif
