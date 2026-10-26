#pragma once

FORCE_INLINE D3DMATRIX TemplateToD3DMatrix44(const Aftk::Matrix44& rhs)
{
	D3DMATRIX m;
	m.m[0][0] = rhs[0][0];m.m[0][1] = rhs[0][1];m.m[0][2] = rhs[0][2];m.m[0][3] = rhs[0][3];
	m.m[1][0] = rhs[1][0];m.m[1][1] = rhs[1][1];m.m[1][2] = rhs[1][2];m.m[1][3] = rhs[1][3];
	m.m[2][0] = rhs[2][0];m.m[2][1] = rhs[2][1];m.m[2][2] = rhs[2][2];m.m[2][3] = rhs[2][3];
	m.m[3][0] = rhs[3][0];m.m[3][1] = rhs[3][1];m.m[3][2] = rhs[3][2];m.m[3][3] = rhs[3][3];

	return m;
}

FORCE_INLINE D3DXMATRIX TemplateToD3DXMatrix44(const Aftk::Matrix44& rhs)
{
	D3DXMATRIX m;
	m.m[0][0] = rhs[0][0];m.m[0][1] = rhs[0][1];m.m[0][2] = rhs[0][2];m.m[0][3] = rhs[0][3];
	m.m[1][0] = rhs[1][0];m.m[1][1] = rhs[1][1];m.m[1][2] = rhs[1][2];m.m[1][3] = rhs[1][3];
	m.m[2][0] = rhs[2][0];m.m[2][1] = rhs[2][1];m.m[2][2] = rhs[2][2];m.m[2][3] = rhs[2][3];
	m.m[3][0] = rhs[3][0];m.m[3][1] = rhs[3][1];m.m[3][2] = rhs[3][2];m.m[3][3] = rhs[3][3];

	return m;
}

FORCE_INLINE Aftk::Matrix44 D3DToTemplateMatrix44(const D3DMATRIX& rhs)
{
	Aftk::Matrix44 m;
	m[0][0] = rhs.m[0][0];m[0][1] = rhs.m[0][1];m[0][2] = rhs.m[0][2];m[0][3] = rhs.m[0][3];
	m[1][0] = rhs.m[1][0];m[1][1] = rhs.m[1][1];m[1][2] = rhs.m[1][2];m[1][3] = rhs.m[1][3];
	m[2][0] = rhs.m[2][0];m[2][1] = rhs.m[2][1];m[2][2] = rhs.m[2][2];m[2][3] = rhs.m[2][3];
	m[3][0] = rhs.m[3][0];m[3][1] = rhs.m[3][1];m[3][2] = rhs.m[3][2];m[3][3] = rhs.m[3][3];

	return m;
}

FORCE_INLINE Aftk::Matrix44 D3DXToTemplateMatrix44(const D3DXMATRIX& rhs)
{
	Aftk::Matrix44 m;
	m[0][0] = rhs.m[0][0];m[0][1] = rhs.m[0][1];m[0][2] = rhs.m[0][2];m[0][3] = rhs.m[0][3];
	m[1][0] = rhs.m[1][0];m[1][1] = rhs.m[1][1];m[1][2] = rhs.m[1][2];m[1][3] = rhs.m[1][3];
	m[2][0] = rhs.m[2][0];m[2][1] = rhs.m[2][1];m[2][2] = rhs.m[2][2];m[2][3] = rhs.m[2][3];
	m[3][0] = rhs.m[3][0];m[3][1] = rhs.m[3][1];m[3][2] = rhs.m[3][2];m[3][3] = rhs.m[3][3];

	return m;
}

FORCE_INLINE D3DXMATRIX NightmareToD3DXMatrix44(const Maths::HMat44& rhs)
{
	return (D3DXMATRIX)rhs;
}

FORCE_INLINE Maths::HMat44 D3DToNightmareMatrix44(const D3DMATRIX& rhs)
{
	return Maths::HMat44(
		Maths::HVec4(rhs.m[0][0], rhs.m[0][1], rhs.m[0][2], rhs.m[0][3]),
		Maths::HVec4(rhs.m[1][0], rhs.m[1][1], rhs.m[1][2], rhs.m[1][3]),
		Maths::HVec4(rhs.m[2][0], rhs.m[2][1], rhs.m[2][2], rhs.m[2][3]),
		Maths::HVec4(rhs.m[3][0], rhs.m[3][1], rhs.m[3][2], rhs.m[3][3]));
}

FORCE_INLINE Maths::HMat44 D3DXToNightmareMatrix44(const D3DXMATRIX& rhs)
{
	return Maths::HMat44(
		Maths::HVec4(rhs.m[0][0], rhs.m[0][1], rhs.m[0][2], rhs.m[0][3]),
		Maths::HVec4(rhs.m[1][0], rhs.m[1][1], rhs.m[1][2], rhs.m[1][3]),
		Maths::HVec4(rhs.m[2][0], rhs.m[2][1], rhs.m[2][2], rhs.m[2][3]),
		Maths::HVec4(rhs.m[3][0], rhs.m[3][1], rhs.m[3][2], rhs.m[3][3]));
}


