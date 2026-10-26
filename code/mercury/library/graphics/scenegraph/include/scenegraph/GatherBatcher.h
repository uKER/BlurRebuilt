#ifndef SCENE_GRAPH_GATHER_BATCHER_H
#define SCENE_GRAPH_GATHER_BATCHER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace SceneGraph
{

class I_View;

class I_GatherBatcher
{
public:
	I_GatherBatcher(const I_View* pView) : m_pView(pView) {}
	virtual ~I_GatherBatcher() {}

	virtual void		EndGather() = 0;

protected:
	const I_View*		m_pView;
};

} // namespace SceneGraph

#endif // SCENE_GRAPH_GATHER_BATCHER_H
