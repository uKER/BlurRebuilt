#ifndef RENDER_DYNAMICEFFECT_TRACER_TRACER_H
#define RENDER_DYNAMICEFFECT_TRACER_TRACER_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "Dynamic/Block.h"

namespace DynamicEffect
{
namespace Tracer
{

class ALIGN16 Tracer : public C_RenderNode
{
public:
	MathsStorage::Float4	m_vColour;
	MathsStorage::Float3	m_vPrevious;
};

}; // namespace Tracer
}; // namespace DynamicEffect

#endif // RENDER_DYNAMICEFFECT_TRACER_TRACER_H
