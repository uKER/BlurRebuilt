#ifndef SCENE_GRAPH_CULLNODE_H
#define SCENE_GRAPH_CULLNODE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "SceneGraph/Bounds.h"

namespace RenderingData
{
	struct Bounds;
};

namespace SceneGraph
{

class C_View;

enum E_BoundsCull
{
	E_BoundsCull_FrustumAxisBox,
	E_BoundsCull_FrustumSphere
};


// todo merge this with the bounds structure
class C_BoundsCull : public C_Bounds
{
public:
	C_BoundsCull();
	~C_BoundsCull() {}

	void						EnableCullFlag(const U32 cullFlag);
	void						DisableCullFlag(const U32 cullFlag);

	const bool					CullFlagEnabled(const U32 renderFlag) const;
	
	S32							Cull(const C_View& view) const;
	S32							CullAxisBox(const C_View& view) const;
	S32							CullAxisBoxDistance(const C_View& view) const;
	S32							CullSphere(const C_View& view) const;
	S32							CullSphereDistance(const C_View& view, const F32 drawDistanceScale) const;
	
protected:
	
	U32							m_CullFlags;
};

} // namespace SceneGraph

#include "CullNode.inl"

#endif // SCENE_GRAPH_CULLNODE_H
