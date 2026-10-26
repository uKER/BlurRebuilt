#ifndef RENDER_DYNAMICEFFECT_LIGHT_INSTANCE_H
#define RENDER_DYNAMICEFFECT_LIGHT_INSTANCE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "Dynamic/Block.h"
#include "Dynamic/Light/Light.h"

namespace DynamicEffect
{
namespace Light
{

class ALIGN16 Instance : public C_RenderNode
{
public:
	MathsStorage::Float2	m_vDimensions;
	F32						m_fIntensity;
	U32						m_iTexture;
	LightHandle				m_hLight;
};

class ALIGN16 Instance2d : public Instance
{
public:
	MathsStorage::Float3	m_vXTransform2d;
	F32						m_fPadding;
	MathsStorage::Float3	m_vYTransform2d;
};

class ALIGN16 Instance3d_P : public Instance
{
public:
	MathsStorage::Float3	m_vPosition;
};

class ALIGN16 Instance3d_P2 : public Instance
{
public:
	MathsStorage::Float3	m_vPosition0;
	F32						m_fPadding;
	MathsStorage::Float3	m_vPosition1;
};

class ALIGN16 Instance3d_P_R : public Instance3d_P
{
public:
	MathsStorage::Float4	m_vRotation;
};

}; // namespace Light
}; // namespace DynamicEffect

#endif // RENDER_DYNAMICEFFECT_LIGHT_INSTANCE_H
