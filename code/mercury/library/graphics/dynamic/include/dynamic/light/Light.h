#ifndef RENDER_DYNAMICEFFECT_LIGHT_LIGHT_H
#define RENDER_DYNAMICEFFECT_LIGHT_LIGHT_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "Core/Containers/Radix.h"
#include "Dynamic/Handle.h"
#include "Dynamic/Light/Enums.h"

namespace Render
{
namespace Occlusion
{
	class C_PercentageQuery;
};
};

namespace DynamicEffect
{

class C_Node;

namespace Light
{

//enum E_Occlusion;

struct ALIGN16 InstanceDefinition
{
	MathsStorage::Float2	m_vSize;
	U32						m_iTexture;
	U32						m_iEffect;
	MathsStorage::Float3	m_vTransform0;
	F32						m_fIntensity;
	MathsStorage::Float3	m_vTransform1;
};

typedef Ut::Vector<InstanceDefinition> InstanceDefinitions;

struct ALIGN16 LightDefinition
{
	MathsStorage::Float3A16	m_vSize;
	E_Occlusion				m_eOcclusion;
	InstanceDefinitions		m_Instances;
};

typedef Ut::Vector<LightDefinition> LightDefinitions;

class ALIGN16 Light : public Ut::ListElementDouble<Light>
{
public:

	Light() : m_iSalt(0), m_fDefaultOcclusion(0.0f), m_VisibilityFlags(0xFFFFFFFF) {}
	
	U32						GetSalt() const	{ return m_iSalt; }

public:

	const LightDefinition *	m_pDefinition;
	const C_Node *			m_pNode;
	U32						m_VisibilityFlags;
	U32						m_iSalt;

	MathsStorage::Float3A16	m_vPosition;
	Maths::RGBA				m_iColour;
	MathsStorage::Float4A16	m_vRotation;
	F32						m_fIntensity;
	F32						m_fOcclusion;
	F32						m_fView;
	F32						m_fDistance;
	MathsStorage::Float4A16	m_vScreenPosition;
	
	typedef Ut::UtVector<Ut::ArrayVector<Render::Occlusion::C_PercentageQuery*, 8> > OcclusionQueries;
	OcclusionQueries		m_OcclusionQueries;

	F32						m_fDefaultOcclusion;
};

typedef C_Handle<Light>		LightHandle;
typedef Ut::UtSet<Light *>	Lights;

struct LightInstance
{
	Light*	pLight;
	U32		sortKey;
};

inline U32 RadixKey(const LightInstance& info)
{
	return info.sortKey;
}

typedef Ut::UtVector<Ut::UtDynamicVector<LightInstance> >	LightArray;
typedef Ut::RadixSort<LightArray>							LightSorter;

}; // namespace Light
}; // namespace DynamicEffect

#endif // RENDER_DYNAMICEFFECT_LIGHT_LIGHT_H
