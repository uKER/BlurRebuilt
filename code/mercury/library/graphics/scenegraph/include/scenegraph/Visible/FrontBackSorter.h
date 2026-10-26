#ifndef SCENE_GRAPH_VISBLE_FRONTBACK_SORTER_H
#define SCENE_GRAPH_VISBLE_FRONTBACK_SORTER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <SceneGraph/Visible/ListSorter.h>
#include <SceneGraph/Visible/Nodes.h>
#include <btl/containers/radix.h>

namespace SceneGraph
{

class C_FrontBackSorter : public I_VisibleNodeListSorter
{
public:
	C_FrontBackSorter();
	virtual ~C_FrontBackSorter() {}

	virtual void Sort(const C_View* pView, C_VisibleNodeList* pVisibleNodeList) const;

private:
	mutable Btl::RadixSorter<Btl::Vector<C_VisibleNode> >	m_Sorter;
};

} // namespace SceneGraph

#endif // SCENE_GRAPH_VISBLE_FRONTBACK_SORTER_H
