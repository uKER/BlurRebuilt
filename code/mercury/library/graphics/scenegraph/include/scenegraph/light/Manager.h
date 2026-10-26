#ifndef SCENE_GRAPH_LIGHT_MANAGER_H
#define SCENE_GRAPH_LIGHT_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include "SceneGraph/GatherNode.h"
#include "SceneGraph/Light/Light.h"
#include "btl/allocators/pool.h"

namespace SceneGraph
{

typedef Btl::List<C_PointLight, Btl::ListParameters<Btl::DoubleEndedList, Btl::MaintainedCounter> >	PointLights;
typedef Btl::List<C_SpotLight, Btl::ListParameters<Btl::DoubleEndedList, Btl::MaintainedCounter> >	SpotLights;
typedef Btl::List<C_DropShadow, Btl::ListParameters<Btl::DoubleEndedList, Btl::MaintainedCounter> >	DropShadows;

class C_LightManager : public I_GatherNode
{
	friend class C_DebugRenderVisitor;
public:

	C_LightManager(C_SceneGraph* pSceneGraph);
	~C_LightManager();

	void				ReservePointLights(const S32 maxPointLights);
	void				ReserveSpotLights(const S32 maxSpotLights);
	void				ReserveDropShadows(const S32 maxSpotLights);

	C_PointLightHandle	AddPointLight(bool isManaged = true);
	void				RemovePointLight(C_PointLightHandle handle);

	C_SpotLightHandle	AddSpotLight(bool isManaged = true);
	void				RemoveSpotLight(C_SpotLightHandle handle);

	C_DropShadowHandle	AddDropShadow(bool isManaged = true);
	void				RemoveDropShadow(C_DropShadowHandle handle);

	Maths::C_RGBA_F32	EvaluateLighting(const Maths::HVec3& vPoint) const;

	// visitor methods
	virtual void		Accept(I_Visitor& visitor) const;

	void				Gather(const C_View& view) const;

#ifndef CFG_GOLD
	void				LogDebug();
#endif

private:
	
	// interface methods
	virtual void		OnGather(const C_View& view) const;

	C_SceneGraph*					m_pSceneGraph;

	Btl::DynamicPool<C_PointLight>	m_PointLightPool;
	PointLights						m_ActivePointLights;
	S32								m_NumPointLights;
	S32								m_MaxPointLights;

	Btl::DynamicPool<C_SpotLight>	m_SpotLightPool;
	SpotLights						m_ActiveSpotLights;
	S32								m_NumSpotLights;
	S32								m_MaxSpotLights;

	Btl::DynamicPool<C_DropShadow>	m_DropShadowPool;
	DropShadows						m_ActiveDropShadows;
	S32								m_NumDropShadows;
	S32								m_MaxDropShadows;

	bool							m_disablePointLightCountWarning;
	bool							m_disableSpotLightCountWarning;
	bool							m_disableDropShadowCountWarning;

};

} // namespace SceneGraph

#endif // SCENE_GRAPH_LIGHT_MANAGER_H
