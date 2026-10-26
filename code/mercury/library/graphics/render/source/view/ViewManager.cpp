#include "stdafx.h"
#include "Render/View/ViewManager.h"

namespace Render
{

C_ViewManager::C_ViewManager() : m_ViewCount(0)
{
}

C_ViewManager::~C_ViewManager()
{
}

C_View* C_ViewManager::GetView(const S32 viewIndex)
{
	return m_Views[viewIndex];
}

S32 C_ViewManager::GetViewCount() const
{
	return m_Views.Size();
}

void C_ViewManager::AddView(C_View* pView)
{
	m_ViewMutex.Wait();

	if (m_Views.Size() == m_ViewCount)
	{
		m_Views.PushBack(pView);
		pView->m_ViewIndex = m_Views.Size() - 1;
		pView->m_View.m_Index = m_Views.Size() - 1;
	}
	else
	{
		for (S32 i = 0; i < m_Views.Size(); ++i)
			if (m_Views[i] == 0)
			{
				m_Views[i] = pView;
				pView->m_ViewIndex = i;
				pView->m_View.m_Index = i;
				break;
			}
	}
	++m_ViewCount;

	m_ViewMutex.Release();
}

void C_ViewManager::RemoveView(C_View* pView)
{
	m_ViewMutex.Wait();

	m_Views[pView->m_ViewIndex] = 0;
	while (m_Views.Size() > 0 && m_Views.Back() == 0)
		m_Views.PopBack();
	--m_ViewCount;

	m_ViewMutex.Release();
}

}; // namespace Render
