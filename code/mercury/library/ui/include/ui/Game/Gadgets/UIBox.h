#ifndef UIBOX_H
#define UIBOX_H

#ifdef _MSC_VER
#pragma once
#endif

#include "ui/Game/Gadgets/Buttons.h"

namespace UI
{

class UIStaticBox : public C_GadgetBase
{
public:
	DECLARE_GADGET( E_UIButtonType_StaticBox, "BOX" )

	void Init();
	void Render();

public:
	F32 m_MaxCorner_x;
	F32 m_MaxCorner_y;

	F32 m_Corner_u;
	F32 m_Corner_v;
};

}; // namespace UI
#endif
