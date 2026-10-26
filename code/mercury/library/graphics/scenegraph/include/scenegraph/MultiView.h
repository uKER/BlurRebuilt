#ifndef SCENE_GRAPH_MULTI_VIEW_H
#define SCENE_GRAPH_MULTI_VIEW_H

#ifdef _MSC_VER
#pragma once
#endif

// remove dependancy
#include "SceneGraph/ViewInterface.h"

namespace SceneGraph
{

class C_View;
class C_SceneGraph;

typedef Btl::Vector<const C_View*, Btl::VectorParameters_Static<6> >	ViewPtrs;

// MultiView combines many Views to allow gathering for all many views simultaneously
class C_MultiView : public I_View
{
public:
	C_MultiView(C_SceneGraph* pSceneGraph);
	~C_MultiView() {}

	bool			AddView(const C_View* pView);
	void			ClearViews();

	const C_View*	GetView(S32 index) const;
	S32				Size() const;
	void			BeginGather() const;
	
	void			EndGather() const;

protected:
	void			ClearNodes() const;
	void			Gather() const;

	ViewPtrs		m_Views;
};

INLINE S32 C_MultiView::Size() const
{
	return m_Views.Size();
}

INLINE const C_View* C_MultiView::GetView(S32 index) const
{
	return m_Views[index];
}

} // namespace SceneGraph

#endif // SCENE_GRAPH_MULTI_VIEW_H
