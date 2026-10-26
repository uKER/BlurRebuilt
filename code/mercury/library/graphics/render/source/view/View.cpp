#include "stdafx.h"
#include "Render/View/View.h"
#include "Render/View/ViewManager.h"

namespace Render
{

C_View::C_View() : m_ViewFlags(0), m_ViewIndex(0)
{
	C_ViewManager::Instance().AddView(this);
}

C_View::~C_View()
{
	C_ViewManager::Instance().RemoveView(this);
}

}; // namespace Render
