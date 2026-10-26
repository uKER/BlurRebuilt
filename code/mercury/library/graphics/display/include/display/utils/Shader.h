#ifndef DISPLAY_UTILS_SHADER_H
#define DISPLAY_UTILS_SHADER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

class C_VSConstant
{
public:

	C_VSConstant() : m_Vector(0.0f, 0.0f, 0.0f, 0.0f) {}
	C_VSConstant(const F32 xyzw) : m_Vector(xyzw, xyzw, xyzw, xyzw) {}
	C_VSConstant(const F32 x, const F32 y, const F32 z = 0.0f, const F32 w = 0.0f) : m_Vector(x, y, z, w) {}

	operator const F32 *() const { return (const F32 *) this; }

public:

	MathsStorage::Float4	m_Vector;

};

class C_PSConstant
{
public:

	C_PSConstant() : m_Vector(0.0f, 0.0f, 0.0f, 0.0f) {}
	C_PSConstant(const F32 xyzw) : m_Vector(xyzw, xyzw, xyzw, xyzw) {}
	C_PSConstant(const F32 x, const F32 y, const F32 z = 0.0f, const F32 w = 0.0f) : m_Vector(x, y, z, w) {}

	operator const F32 *() const { return (const F32 *) this; }

public:

	MathsStorage::Float4	m_Vector;

};

}; // namespace Display

#endif // DISPLAY_UTILS_SHADER_H
