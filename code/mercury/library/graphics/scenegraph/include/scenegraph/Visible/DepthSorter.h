#ifndef SCENE_GRAPH_VISBLE_DEPTH_SORTER_H
#define SCENE_GRAPH_VISBLE_DEPTH_SORTER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <SceneGraph/Visible/ListSorter.h>
#include <SceneGraph/Visible/Nodes.h>
#include <btl/containers/radix.h>

namespace SceneGraph
{

enum E_DepthSortOrder
{
	E_DepthSortOrder_Default,
	E_DepthSortOrder_FrontBack,
	E_DepthSortOrder_BackFront
};

enum E_DepthSortMethod
{
	E_DepthSortMethod_All,
	E_DepthSortMethod_PerType
};

class C_DepthSorter : public I_VisibleNodeListSorter
{
public:
	C_DepthSorter();
	C_DepthSorter(E_DepthSortOrder order, E_DepthSortMethod method);
	virtual ~C_DepthSorter() {}

	virtual void Sort(const C_View* pView, C_VisibleNodeList* pVisibleNodeList) const;

	void	SetSortOrder(E_DepthSortOrder order) { m_Order = order; }
	void	SetSortMethod(E_DepthSortMethod method) { m_Method = method; }

	void	SetPrimitiveSorting(const bool primitives) { m_Primitives = primitives; }
	void	SetInstanceSorting(const bool instances) { m_Instances = instances; }
	void	SetModelInstanceSorting(const bool modelInstances) { m_ModelInstances = modelInstances; }
	void	SetAbstractSorting(const bool abstracts) { m_Abstracts = abstracts; }

private:
	mutable Btl::RadixSorter<Btl::Vector<C_VisibleNode> >	m_Sorter;

	E_DepthSortOrder		m_Order;
	E_DepthSortMethod		m_Method;
	bool					m_Primitives;
	bool					m_Instances;
	bool					m_ModelInstances;
	bool					m_Abstracts;
};

} // namespace SceneGraph

#endif // SCENE_GRAPH_VISBLE_FRONTBACK_SORTER_H
