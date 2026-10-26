#ifndef RENDER_VIEW_MANAGER_H
#define RENDER_VIEW_MANAGER_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "Btl/Containers/Vector.h"
#include "Render/View/View.h"
#include "Core/Utility/Singleton.h"

namespace Render
{

class C_ViewManager : public Ut::Singleton<C_ViewManager>
{
public:

	C_ViewManager();
	~C_ViewManager();

	C_View*	GetView(const S32 viewIndex);
	S32		GetViewCount() const;

	void	AddView(C_View* pView);
	void	RemoveView(C_View* pView);

private:

	typedef Btl::Vector<C_View*> Views;
	Views			m_Views;
	U32				m_ViewCount;
	MT::Mutex		m_ViewMutex;

};

}; // namespace Render

#endif // RENDER_VIEW_MANAGER_H
