#ifndef DISPLAY_D3D10VERTEXDECLARATION_H
#define DISPLAY_D3D10VERTEXDECLARATION_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Buffer/VertexFormat.h"
#include "Display/Device/D3D10/D3D10.h"

namespace NewDisplay
{
namespace D3D10
{

class C_VertexDeclaration
{
public:

	C_VertexDeclaration();
	~C_VertexDeclaration();

	const bool IsCreated() const;

public:
	
	ID3D10InputLayout* m_pd3dInputLayout;
};

#include "D3D10VertexDeclaration.inl"

}; // namespace D3D10
}; // namespace NewDisplay

#endif // DISPLAY_D3D10VERTEXDECLARATION_H
