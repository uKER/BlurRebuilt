#include "stdafx.h"
#include "Gui.h"
#include "Camera.h"
#include <display/render/BasicRender.h>
#include <render/font/Font.h>

C_Gui::C_Gui(C_GuiRender& r):
	m_render(r),
	m_active(0),
	m_hot(0),
	m_groupIndex(0)
{
	SetAutoWidth();
	SetSpacing(1.f);
	SetIndentSize(20.f);
}

void C_Gui::SetFixedWidth(F32 fixedWidth)
{
	m_fixedWidth = fixedWidth;
	m_autoWidth = false;
}

void C_Gui::SetAutoWidth()
{
	m_autoWidth = true;
}

void C_Gui::SetSpacing(F32 spacing)
{
	m_spacing = spacing;
}

void C_Gui::SetIndentSize(F32 s)
{
	m_indentSize = s;
}

void C_Gui::SetWindowSize(F32 w, F32 h)
{
	m_window = C_Rect(0.f, 0.f, w, h);
}

void C_Gui::Begin()
{
	if (mouse.buttons.down)
	{
		mouse.dragX = mouse.x;
		mouse.dragY = mouse.y;
	}
	m_hot = m_hotThisFrame;
	m_hotThisFrame = 0;
	
	m_groupIndex = 0;
	m_groupStack[0].rect = C_Rect(0.f, 0.f, 0.f, 0.f);
	m_groupStack[0].flags = 0;
}

void C_Gui::End()
{
	//m_render.DrawFrame(m_groupStack[0].rect);

	mouse.buttons.down = false;
	mouse.buttons.up = false;
	mouse.prevX = mouse.x;
	mouse.prevY = mouse.y;
	
	BIZ_ASSERT(m_groupIndex == 0);
}

void C_Gui::BeginGroup(U32 flags, F32 x, F32 y, F32 w, F32 h)
{
	++m_groupIndex;
	BIZ_ASSERT(m_groupIndex < kMaxGroups);
	const Group& parentGroup = m_groupStack[m_groupIndex - 1];
	Group& newGroup = m_groupStack[m_groupIndex];
	
	C_Rect r;
	r.w = 0.f;
	r.h = 0.f;
	
	if (parentGroup.flags & GroupFlags::LayoutHorizontal)
	{
		if (parentGroup.flags & GroupFlags::AlignRight)
		{
			r.x = parentGroup.rect.x - w;
		}
		else
		{
			r.x = parentGroup.rect.x + parentGroup.rect.w;
		}
		
		if (parentGroup.flags & GroupFlags::AlignTop)
		{
			r.y = parentGroup.rect.y;
		}
		else //if (group.flags & GroupFlags::AlignBottom)
		{
			r.y = parentGroup.rect.y - h;
		}
	}
	else if (parentGroup.flags & GroupFlags::LayoutVertical)
	{
		if (parentGroup.flags & GroupFlags::AlignTop)
		{
			r.y = parentGroup.rect.y + parentGroup.rect.h;
		}
		else // if (group.flags & GroupFlags::AlignBottom)
		{
			r.y = parentGroup.rect.y - h;
		}

		if (parentGroup.flags & GroupFlags::AlignRight)
		{
			r.x = parentGroup.rect.x - w;
		}
		else
		{
			r.x = parentGroup.rect.x; // + parentGroup.rect.w;
		}
	}
	else
	{
		r.x = 0.f;
		r.y = 0.f;
	}
	
	r.x += x;
	r.y += y;
	
	newGroup.rect = r;
	newGroup.flags = flags;
}

void C_Gui::EndGroup()
{
	BIZ_ASSERT(m_groupIndex > 0);
	
	Group& parentGroup = m_groupStack[m_groupIndex - 1];
	const Group& childGroup = m_groupStack[m_groupIndex];
	
	// TODO: take into account offsets?
	#if 0
	
	if (parentGroup.flags & GroupFlags::LayoutHorizontal)
	{
		if (parentGroup.flags & GroupFlags::AlignRight)
		{
			// grow left
			parentGroup.rect.x -= childGroup.rect.w;
			parentGroup.rect.w += childGroup.rect.w;
		}
		else
		{
			// grow right
			//parentGroup.rect.w += Maths::Max(childchildGroup.rect.w;
		}
		
		if (parentGroup.flags & GroupFlags::AlignTop)
		{
			//r.y = parentGroup.rect.y;
		}
		else //if (group.flags & GroupFlags::AlignBottom)
		{
			//r.y = parentGroup.rect.y - h;
		}
	}
	else
	{
		if (parentGroup.flags & GroupFlags::AlignTop)
		{
			parentGroup.rect.h = Maths::Max(parentGroup.rect.h 
			r.y = parentGroup.rect.y + parentGroup.rect.h;
		}
		else // if (group.flags & GroupFlags::AlignBottom)
		{
			r.y = parentGroup.rect.y - h;
		}

		if (parentGroup.flags & GroupFlags::AlignRight)
		{
			r.x = parentGroup.rect.x + parentGroup.rect.w - w;
		}
		else
		{
			r.x = parentGroup.rect.x + parentGroup.rect.w;
		}
	}
	#endif
	
	//m_render.DrawFrame(childGroup.rect);
	
	parentGroup.rect.Enclose(childGroup.rect);
	
	m_groupIndex--;
}

void C_Gui::BeginChildren()
{
	const Group& g = m_groupStack[m_groupIndex];

	BeginGroup(GroupFlags::LayoutVertical | GroupFlags::AlignLeft | GroupFlags::AlignTop, m_indentSize, 0.f);
}

void C_Gui::EndChildren()
{
	EndGroup();
}

bool C_Gui::Button(U32 id, const CHAR8* caption)
{
	F32 w, h;
	m_render.MeasureButton(caption, w, h);
	
	C_Rect r = PlaceRect(w, h);
	bool result = DoButton(id, r);
	m_render.DrawButton(r, caption, IsActive(id), IsHot(id));
	return result;
}

bool C_Gui::CheckBox(U32 id, const CHAR8* caption, bool enabled)
{
	F32 w, h;
	m_render.MeasureCheckBox(caption, w, h);
	
	C_Rect r = PlaceRect(w, h);
	bool result = DoButton(id, r);
	if (result)
	{
		enabled = !enabled;
	}
	
	m_render.DrawCheckBox(r, caption, enabled, IsActive(id), IsHot(id));
	
	
	return enabled;
}

void C_Gui::Label(const CHAR8* text)
{
	F32 w, h;
	m_render.MeasureLabel(text, w, h);
	
	C_Rect r = PlaceRect(w, h);
	
	m_render.DrawLabel(r, text, false);
}

bool C_Gui::Camera(U32 id, F32 x, F32 y, F32 w, F32 h, C_Camera& camera)
{
	if (IsActive(id))
	{
		// Being manipulated
		if (mouse.buttons.up)
		{
			// finished.
			camera.DisableAllInputs();
			ClearActive();
		}
		else
		{
			// update camera according to mouse movement.
			F32 dx = mouse.x - mouse.prevX;
			F32 dy = mouse.y - mouse.prevY;
		
			F32 sens = 0.4f;
			F32 moveX = dx * sens;
			F32 moveY = -dy * sens;
			
			if (mouse.buttons.left)
			{
				// Rotation.
				if(moveX != 0.f)
					camera.EnableInput(E_CameraInput_RotateLR, -moveX);
				else
					camera.DisableInput(E_CameraInput_RotateLR);

				if(moveY != 0.f)
					camera.EnableInput(E_CameraInput_RotateUD, -moveY);
				else
					camera.DisableInput(E_CameraInput_RotateUD);
			}
			else if (mouse.buttons.middle)
			{
				// Translation.
				if(moveX != 0.f)
					camera.EnableInput(E_CameraInput_MoveLR, moveX);
				else
					camera.DisableInput(E_CameraInput_MoveLR);

				if(moveY != 0.f)
					camera.EnableInput(E_CameraInput_MoveUD, moveY);
				else
					camera.DisableInput(E_CameraInput_MoveUD);
			}
			else if (mouse.buttons.right)
			{
				// Zoom.
				if(moveY != 0.f)
					camera.EnableInput(E_CameraInput_Zoom, moveY);
				else
					camera.DisableInput(E_CameraInput_Zoom);
			}
		}
	}
	else if (IsHot(id))
	{
		if (mouse.buttons.down)
		{
			SetActive(id);
		}
	}
	
	C_Rect r(x, y, w, h);
	
	if (r.Inside(mouse.x, mouse.y))
	{
		SetHot(id);
	}

	return true;
}

bool C_Gui::TreeNode(U32 id, const CHAR8* caption, bool openByDefault)
{
	bool open = openByDefault;
	
	IdToStateMap::InsertionReturnType result = m_controlStates.Insert(id, openByDefault);
	
	if (!result.inserted)
	{
		open = (((result.iterator->GetValue()) & 1) != 0);
	}
	
	F32 w, h;
	m_render.MeasureTreeNode(caption, w, h);
	
	C_Rect r = PlaceRect(w, h);
	
	bool clicked = DoButton(id, r);
	
	if (clicked)
	{
		open = !open;
		
		if (open)
		{
			result.iterator->GetValue() |= 1;
		}
		else
		{
			result.iterator->GetValue() &= ~1;
		}
	}
	
	m_render.DrawTreeNode(r, caption, open, IsActive(id), IsHot(id));
	
	return open;
}

void C_Gui::ColourSwatch(F32 w, F32 h, const Maths::RGBA& colour)
{
	C_Rect r = PlaceRect(w, h);
	m_render.DrawColourSwatch(r, colour);
}

U32 C_Gui::ComboBox(U32 id, const CHAR8** options, U32 numOptions, U32 selected)
{
	// TODO: need to make it overlap rest of ui.
	return selected;
}

F32 C_Gui::HorizontalSlider(U32 id, F32 width, F32 minValue, F32 maxValue, F32 currentValue)
{
	F32 height = 16.f;
	width = Maths::Max(width, 1.f);
	C_Rect r = PlaceRect(width, height);
	
	F32 newValue = currentValue;

	if (IsActive(id))
	{
		// set slider pos according to current mouse position.
		// TODO: not quite right
		F32 relx = (mouse.x - r.x);
		newValue = (relx / width) * (maxValue - minValue);
		newValue = Maths::Clamped(newValue, minValue, maxValue);
	
		if (mouse.buttons.up)
		{
			ClearActive();
		}
	}
	else if (IsHot(id))
	{
		if (mouse.buttons.down)
		{
			SetActive(id);
		}
	}
	
	if (r.Inside(mouse.x, mouse.y))
	{
		SetHot(id);
	}
	
	m_render.DrawHorizontalSlider(r, (currentValue - minValue) / (maxValue - minValue), IsHot(id), IsActive(id));
	
	return newValue;
}

bool C_Gui::DoButton(U32 id, const C_Rect& r)
{
	bool result = false;

	if (IsActive(id))
	{
		if (mouse.buttons.up)
		{
			if (IsHot(id))
			{
				result = true;
			}
			ClearActive();
		}
	}
	else if (IsHot(id))
	{
		if (mouse.buttons.down)
		{
			SetActive(id);
		}
	}
	
	if (r.Inside(mouse.x, mouse.y))
	{
		SetHot(id);
	}
	return result;
}

bool C_Gui::IsActive(U32 id) const
{
	return m_active == id;
}

bool C_Gui::IsHot(U32 id) const
{
	return m_hot == id;
}

void C_Gui::SetHot(U32 id)
{
	if (m_active == 0 || m_active == id)
	{
		m_hotThisFrame = id;
	}
}

void C_Gui::SetActive(U32 id)
{
	m_active = id;
}

void C_Gui::ClearActive()
{
	m_active = 0;
}

C_Rect C_Gui::PlaceRect(F32 w, F32 h)
{
	Group& group = m_groupStack[m_groupIndex];
	F32 x = group.rect.x;
	F32 y = group.rect.y;
	
	if (!m_autoWidth)
	{
		w = m_fixedWidth;
	}
	
	if (group.flags & GroupFlags::LayoutHorizontal)
	{
		if (group.flags & GroupFlags::AlignRight)
		{
			group.rect.x -= w;
			x = group.rect.x;
		}
		else
		{
			x = group.rect.x + group.rect.w;
		}
		group.rect.w += w;
		
		if (group.flags & GroupFlags::AlignTop)
		{
			y = group.rect.y;
			group.rect.h = Maths::Max(group.rect.h, h);
		}
		else if (group.flags & GroupFlags::AlignBottom)
		{
			if (h > group.rect.h)
			{
				group.rect.y -= (h - group.rect.h);
				group.rect.h = h;
			}
			y = group.rect.y + group.rect.h - h;
		}
	}
	else
	{
		if (group.flags & GroupFlags::AlignTop)
		{
			y = group.rect.y + group.rect.h;
		}
		else // if (group.flags & GroupFlags::AlignBottom)
		{
			group.rect.y -= h;
			y = group.rect.y;
		}
		group.rect.h += h;

		if (group.flags & GroupFlags::AlignRight)
		{
			if (w > group.rect.w)
			{
				group.rect.x -= (w - group.rect.w);
				group.rect.w = w;
			}
			x = group.rect.x + group.rect.w - w;
		}
		else
		{
			x = group.rect.x;
			if (w > group.rect.w)
			{
				group.rect.w = w;
			}
		}
	}
	
	return C_Rect(x, y, w, h);
}

//
// C_GuiRender
//

C_GuiRender::C_GuiRender()
{
	m_font = Render::Font::FontSystem()->GetFontRenderer("dontship/fonts/arialdebug.font");
	BIZ_ASSERT(m_font);
	if (m_font)
	{
		m_font->SetScaleX(16);
		m_font->SetScaleY(16);
		m_font->SetShadowEnable( true );
		//m_font->SetColourOutline(Maths::RGBA(0,0,25,255));
		//m_font->SetOutlineSize(0.35f);
	}
	
	m_hotTextColour = Maths::RGBA(255, 255, 255);
	m_textColour = Maths::RGBA(230, 230, 230);
	m_activeTextColour = Maths::RGBA(255, 229, 184);
}

void C_GuiRender::MeasureText(const CHAR8* str, F32& outW, F32& outH)
{
	F32 w = 0.f;
	F32 h = 0.f;
	
	if (str)
	{
		const U32 wrapWidth = 0;
		Maths::HVec2 size = m_font->GetDimensions(wrapWidth, str);
		w = size.X();
		h = size.Y();
	}
	
	outW = w;
	outH = h;
}

const F32 kButtonPadding = 3.f;
const F32 kLabelPaddingX = 2.f;
const F32 kLabelPaddingY = 1.f;
const F32 kCheckBoxPadding = 2.f;

void C_GuiRender::MeasureButton(const CHAR8* str, F32& outW, F32& outH)
{
	F32 w, h;
	MeasureText(str, w, h);

	F32 kPadding = kButtonPadding * 2.f;
	
	w += kPadding;
	h += kPadding;
	
	outW = w;
	outH = h;
}

void C_GuiRender::MeasureLabel(const CHAR8* str, F32& outW, F32& outH)
{
	F32 w, h;
	MeasureText(str, w, h);

	w += kLabelPaddingX * 2.f;
	h += kLabelPaddingY * 2.f;
	
	outW = w;
	outH = h;
}

void C_GuiRender::MeasureCheckBox(const CHAR8* str, F32& outW, F32& outH)
{
	F32 w, h;
	MeasureText(str, w, h);
	
	F32 inset = 3.f;
	
	F32 checkBoxSize = Maths::Max(h - inset * 2.f, 0.f);

	w += checkBoxSize + inset * 2.f + kLabelPaddingX;
	h += kLabelPaddingY * 2.f;
	
	outW = w;
	outH = h;
}

void C_GuiRender::MeasureTreeNode(const CHAR8* str, F32& outW, F32& outH)
{
	MeasureCheckBox(str, outW, outH);
}

void C_GuiRender::DrawLabel(const C_Rect& r, const CHAR8* str, bool hot)
{
	//Display::BasicRender()->DrawColoured2DRect(r.x, r.y, r.w, r.h, Maths::RGBA(0, 0, 0, 0x80));
	Maths::HVec2 pos(r.x + kLabelPaddingX, r.y + kLabelPaddingY);
	m_font->SetColourTop(m_textColour);
	m_font->SetColourBottom(m_textColour);
	m_font->Print(pos, str);
}

void C_GuiRender::DrawButton(const C_Rect& r, const CHAR8* str, bool active, bool hot)
{
	U32 alpha = 0x40;
	Maths::RGBA colour(128, 128, 128, alpha);
	if (hot)
	{
		if (active)
		{
			colour = Maths::RGBA(0xff, 0xff, 0xcc, alpha);
		}
		else
		{
			colour = Maths::RGBA(0xcc, 0xcc, 0xcc, alpha);
		}
	}
	Display::BasicRender()->DrawColoured2DRect(r.x, r.y, r.w, r.h, colour);
	
	Maths::HVec2 pos(r.x + r.w / 2.f, r.y + (r.h - m_font->GetHeight()) / 2.f);
	
	SetTextColour(hot, active);
	m_font->Print(pos, str, Render::Font::E_Justification_Centre);
}

void C_GuiRender::DrawCheckBox(const C_Rect& r, const CHAR8* str, bool enabled, bool active, bool hot)
{
	U32 alpha = 0xa0;
	Maths::RGBA colour(0xd0, 0xc0, 0xc0, alpha);
	if (hot)
	{
		if (active)
		{
			colour = Maths::RGBA(0xff, 0xff, 0xcc, alpha);
		}
		else
		{
			colour = Maths::RGBA(0xd0, 0xd0, 0xd0, alpha);
		}
	}
	
	F32 inset = 3.f;
	F32 boxSize = Maths::Max(r.h - inset * 2.f, 0.f);
	F32 boxX = r.x + inset;
	F32 boxY = r.y + inset;
	
	const F32 border = 1.f;
	
	Maths::RGBA borderColour(32, 32, 32, alpha);
	
	Display::BasicRender()->DrawColoured2DRect(boxX, boxY, boxSize, boxSize, colour);
	Display::BasicRender()->DrawColoured2DRect(boxX + border, boxY + border, boxSize - 2.f * border, boxSize - 2.f * border, borderColour);
	
	if (enabled)
	{
		const F32 i = 2.f;
		Maths::RGBA crossColour(0xff, 0xff, 0xff, 0xdd);
		Display::BasicRender()->DrawColoured2DLine(boxX + i, boxY + i, boxX + boxSize - i, boxY + boxSize - i, crossColour);
		Display::BasicRender()->DrawColoured2DLine(boxX + i, boxY + boxSize - i, boxX + boxSize - i, boxY + i, crossColour);
	}
	
	Maths::HVec2 pos(r.x + inset * 2.f + boxSize, r.y + (r.h - m_font->GetHeight()) / 2.f);
	
	SetTextColour(hot, active);
	m_font->Print(pos, str);
}


void C_GuiRender::DrawTreeNode(const C_Rect& r, const CHAR8* str, bool open, bool active, bool hot)
{
	U32 alpha = 0xc0;
	Maths::RGBA colour(0xa0, 0xa0, 0xa0, alpha);
	if (hot)
	{
		if (active)
		{
			colour = Maths::RGBA(0xff, 0xff, 0xcc, alpha);
		}
		else
		{
			colour = Maths::RGBA(0xcc, 0xcc, 0xcc, alpha);
		}
	}
	
	
	
	F32 inset = 3.f;
	
	F32 tsize = Maths::Max(r.h - inset * 2.f, 0.f);
	F32 tx = r.x + inset;
	F32 ty = r.y + inset;
	
	if (open)
	{
		Display::BasicRender()->DrawColoured2DTri(tx, ty, tx + tsize, ty, tx + tsize / 2.f, ty + tsize, colour);
	}
	else
	{
		Display::BasicRender()->DrawColoured2DTri(tx, ty, tx + tsize, ty + tsize / 2.f, tx, ty + tsize, colour);
	}
	
	Maths::HVec2 pos(r.x + inset * 2.f + tsize, r.y + (r.h - m_font->GetHeight()) / 2.f);
	
	SetTextColour(hot, active);
	m_font->Print(pos, str);
}

void C_GuiRender::DrawColourSwatch(const C_Rect& r, const Maths::RGBA& colour)
{
	Maths::RGBA outlineColour(255, 255, 255, 0x80);
	F32 inset = 1.f;
	Display::BasicRender()->DrawColoured2DRect(r.x, r.y, r.w, r.h, outlineColour);
	Display::BasicRender()->DrawColoured2DRect(r.x + inset, r.y + inset, r.w - 2.f * inset, r.h - 2.f * inset, colour);
}

void C_GuiRender::DrawFrame(const C_Rect& r)
{
	Maths::RGBA frameColour(0x80, 0xff, 0x80, 0xdd);
	
	Display::BasicRender()->DrawColoured2DLine(r.x, r.y, r.x + r.w, r.y, frameColour);
	Display::BasicRender()->DrawColoured2DLine(r.x + r.w, r.y, r.x + r.w, r.y + r.h, frameColour);
	Display::BasicRender()->DrawColoured2DLine(r.x + r.w, r.y + r.h, r.x, r.y + r.h, frameColour);
	Display::BasicRender()->DrawColoured2DLine(r.x, r.y + r.h, r.x, r.y, frameColour);
}

void C_GuiRender::DrawHorizontalSlider(const C_Rect& r, F32 fraction, bool hot, bool active)
{
	Maths::RGBA bg(0x80, 0x80, 0x80, 0x80);
	Display::BasicRender()->DrawColoured2DRect(r.x, r.y, r.w, r.h, bg);
	
	Maths::RGBA nubColour = m_textColour;
	if (active)
	{
		nubColour = m_activeTextColour;
	}
	else if (hot)
	{
		nubColour = m_hotTextColour;
	}
	F32 inset = 1.f;
	F32 nubH = Maths::Max(r.h - 2.f * inset, 0.f);
	F32 nubW = Maths::Min(nubH, r.w);
		
	Display::BasicRender()->DrawColoured2DRect(r.x + ((r.w - nubW) * fraction), r.y + inset, nubW, nubH, nubColour);
}

void C_GuiRender::SetTextColour(bool hot, bool active)
{
	Maths::RGBA top;
	Maths::RGBA bot;
	if (active && hot)
	{
		top = bot = m_activeTextColour;
	}
	else if (hot)
	{
		top = bot = m_hotTextColour;
	}
	else
	{
		top = bot = m_textColour;	
	}
	
	m_font->SetColourTop(top);
	m_font->SetColourBottom(bot);
}