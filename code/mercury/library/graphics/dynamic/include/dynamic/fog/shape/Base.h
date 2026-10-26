#ifndef RENDER_DYNAMICEFFECT_FOG_SHAPE_SIMPLESHAPE_H
#define RENDER_DYNAMICEFFECT_FOG_SHAPE_SIMPLESHAPE_H

#if _MSC_VER > 1000
#pragma once
#endif

#include "Display/Buffer/VertexDeclarations.h"
#include "Dynamic/Fog/Enums.h"

namespace DynamicEffect
{
namespace Fog
{

class ShapeBase
{
public:
	Display::C_VertexBuffer		m_VertexBuffer;
	Display::C_IndexBuffer		m_IndexBuffer;
	U32							m_VertexCount;
	U32							m_IndexCount;
	Display::E_PrimitiveType	m_PrimitiveType;

	E_Shape						m_eShape;
};

}; // namespace Fog
}; // namespace DynamicEffect

#endif // RENDER_DYNAMICEFFECT_FOG_SHAPE_SIMPLESHAPE_H
