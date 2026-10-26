#ifndef SCENE_GRAPH_LIGHT_H
#define SCENE_GRAPH_LIGHT_H

#ifdef _MSC_VER
#pragma once
#endif

#include "SceneGraph/GatherNode.h"
#include "SceneGraph/Handle.h"
#include "SceneGraph/Light/Enums.h"

namespace SceneGraph
{
class C_VisibleDetect;
class C_View;

class C_Light : public I_GatherNode
{
	friend class C_LightManager;
public:
	C_Light();
	~C_Light();
public:
	// set accessors
	void	SetColour(const Maths::C_RGBA_F32& colour)			{ Maths::Store(Maths::HVec3(colour.m_V), &m_Colour); }
	void	SetColour(const F32 r, const F32 g, const F32 b)	{ m_Colour.x = r; m_Colour.y = g; m_Colour.z = b; }
	void	SetCullDistance(const F32 cullDistance)				{ m_CullDistance = cullDistance; }
	void	SetNearCull(const F32 nearCull)						{ m_NearCull = nearCull; }
	void	SetFlags(const U32 flags)							{ m_Flags = flags; }
	void	SetVisDetect(SceneGraph::C_VisibleDetect* pVisDetect)	{ m_pVisDetect = pVisDetect; }
	void	SetVisibilityFlags(const U32 visibilityFlags)		{ m_VisibilityFlags = visibilityFlags;}
	// get accessors
	const MathsStorage::Float4&	GetBoundingSphere() const		{ return m_BoundingSphere; }

	void AllocatePerViewIntensities(S32 numPerViewIntensities);
	void SetPerViewIntensity(S32 viewType, F32 intensity);

public:
	MathsStorage::Float3	m_Position;
	F32						m_CullDistance;
	MathsStorage::Float3	m_Colour;
	F32						m_NearCull;
	U16						m_RenderFlags;
	U16						m_ShadowFlags;
	U32						m_Salt;
	const void*				m_pData;
	U32						m_Flags;
protected:
	MathsStorage::Float4	m_BoundingSphere;
	bool					m_IsManaged;
	SceneGraph::C_VisibleDetect*	m_pVisDetect;
	U32						m_VisibilityFlags;
	Btl::Vector<F32>		m_PerViewIntensities;
	void					SetVisibility(const U32 iViewBitfield) const;
};

class C_PointLight : public C_Light, public Btl::ListElementDouble<C_PointLight>
{
public:
	C_PointLight();
	~C_PointLight();
	void					Gather(const C_View& view) const;
	// visitor methods
	virtual void			Accept(I_Visitor& visitor) const;
	void					DebugRender(const C_View& view) const;
private:
	// interface methods
	virtual void			OnGather(const C_View& view) const;	
public:
	void					SetDimensions(const Maths::HVec3 position, const F32 radius);
public:
	F32						m_Radius;
};

class C_SpotLight : public C_Light, public Btl::ListElementDouble<C_SpotLight>
{
public:
	C_SpotLight();
	~C_SpotLight();
	void					Gather(const C_View& view) const;
	// visitor methods
	virtual void			Accept(I_Visitor& visitor) const;
	void					DebugRender(const C_View& view) const;
private:
	// interface methods
	virtual void			OnGather(const C_View& view) const;	
public:
	void					SetDimensions(const Maths::HMat43& worldMatrix, const F32 radius, const F32 horizontalFoV, const F32 verticalFoV);
public:
	MathsStorage::MatrixRow44	m_Matrix;
	F32							m_Radius;
	F32							m_HorizontalFoV;
	F32							m_VerticalFoV;
};

class C_DropShadow : public I_GatherNode, public Btl::ListElementDouble<C_DropShadow>
{
	friend class C_LightManager;
public:
	C_DropShadow();
	~C_DropShadow();

	// set accessors
	void	SetIntensity(const F32& intensity)					{ m_ShadowIntensity = intensity; }
	void	SetCullDistance(const F32 cullDistance)				{ m_CullDistance = cullDistance; }
	void	SetCullScreenSize(const F32 cullScreenSize)			{ m_CullScreenSize = cullScreenSize; }
	// get accessors
	const MathsStorage::Float4&	GetBoundingSphere() const		{ return m_BoundingSphere; }

	void					Gather(const C_View& view) const;
	// visitor methods
	virtual void			Accept(I_Visitor& visitor) const;
private:
	// interface methods
	virtual void			OnGather(const C_View& view) const;	
public:
	void					SetDimensions(const Maths::HVec3 position, const F32 radius);
public:
	MathsStorage::Float3	m_Position;
	F32						m_CullDistance;
	F32						m_CullScreenSize;
	F32						m_ShadowIntensity;
	F32						m_Radius;
	U32						m_Salt;
protected:
	MathsStorage::Float4	m_BoundingSphere;
	bool					m_IsManaged;
};

template <typename T>
class C_VisibleObject
{
public:
	const T*	m_pObject;
	F32			m_Intensity;
	F32			m_ScreenSize;
	U32			m_SortKey;
};

typedef C_VisibleObject<C_PointLight>		C_VisiblePointLight;
typedef C_VisibleObject<C_SpotLight>		C_VisibleSpotLight;
typedef C_VisibleObject<C_DropShadow>		C_VisibleDropShadow;

typedef Btl::Vector<C_VisiblePointLight>	C_VisiblePointLightList;
typedef Btl::Vector<C_VisibleSpotLight>		C_VisibleSpotLightList;
typedef Btl::Vector<C_VisibleDropShadow>	C_VisibleDropShadowList;

typedef C_Handle<C_PointLight>				C_PointLightHandle;
typedef C_Handle<C_SpotLight>				C_SpotLightHandle;
typedef C_Handle<C_DropShadow>				C_DropShadowHandle;

} // namespace SceneGraph

#endif // SCENE_GRAPH_LIGHT_H
