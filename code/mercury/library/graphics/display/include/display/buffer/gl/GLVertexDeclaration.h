#ifndef DISPLAY_GLVERTEXDECLARATION_H
#define DISPLAY_GLVERTEXDECLARATION_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Resource/Resource.h"
#include "Display/Buffer/VertexFormat.h"
#include "Display/Device/GL/GL.h"
#include "Core/Containers.h"

namespace NewDisplay
{
namespace GL
{

class C_VertexDeclaration : public C_Resource
{
public:

	C_VertexDeclaration();
	~C_VertexDeclaration();

	const bool IsCreated() const;

public:

	Ut::UtVector<Ut::DynamicVector<C_VertexElement> > m_Elements;
};

#include "GLVertexDeclaration.inl"

}; // namespace GL
}; // namespace NewDisplay

#endif // DISPLAY_GLVERTEXDECLARATION_H
