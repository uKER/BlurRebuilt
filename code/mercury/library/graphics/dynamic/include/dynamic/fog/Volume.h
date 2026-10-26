#ifndef RENDER_DYNAMICEFFECT_FOG_VOLUME_H
#define RENDER_DYNAMICEFFECT_FOG_VOLUME_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "Dynamic/Block.h"

namespace DynamicEffect
{
namespace Fog
{

class Volume : public C_RenderNode
{
public:
	MathsStorage::Float4A16	m_vRotation;
	MathsStorage::Float4A16	m_vColour;
	MathsStorage::Float2A16	m_vSettings;
};

typedef Ut::Vector<Volume*> Volumes;

}; // namespace Fog
}; // namespace DynamicEffect

#endif
