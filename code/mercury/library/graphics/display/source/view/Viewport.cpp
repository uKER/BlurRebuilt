#include "stdafx.h"
#include "Display/View/Viewport.h"

namespace Display
{

C_Viewport::C_Viewport() :
	m_X(0),
	m_Y(0),
	m_W(1),
	m_H(1),
	m_MinZ(0.f),
	m_MaxZ(1.f)
{
}

C_Viewport::~C_Viewport()
{
}

}; // namespace Display
