#ifndef SCENE_GRAPH_VISBLE_LISTSORTER_H
#define SCENE_GRAPH_VISBLE_LISTSORTER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace SceneGraph
{

class C_View;
struct C_VisibleNodeList;

class I_VisibleNodeListSorter
{
public:
	I_VisibleNodeListSorter() : m_VolatileInfoFlags(0) {}
	virtual ~I_VisibleNodeListSorter() {}

	virtual void Sort(const C_View* pView, C_VisibleNodeList* pVisibleNodeList) const {};

	const U32	GetVolatileInfoFlags() const { return m_VolatileInfoFlags; }

protected:
	U32			m_VolatileInfoFlags;
};

} // namespace SceneGraph

#endif // SCENE_GRAPH_VIEW_H
