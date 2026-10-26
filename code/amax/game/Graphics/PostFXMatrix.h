#ifndef AMAX_GRAPHICS_PostFxMatrix_H
#define AMAX_GRAPHICS_PostFxMatrix_H

#include <Xt/XtGame.h>

namespace Game
{

struct PostFxMatrix
{
	PostFxMatrix() { m_pOut = 0; m_bEnabled = false; }
	virtual Maths::Mat44 Matrix() const;

	PostFxMatrix*	m_pOut;
	bool			m_bEnabled;
};

struct PostFxMatrixRoot : public PostFxMatrix
{
};

struct PostFxMatrixBrightness : public PostFxMatrix
{
	PostFxMatrixBrightness() { m_fBrightness = 0.f; }
	virtual Maths::Mat44 Matrix() const;

	F32		m_fBrightness;
};

struct PostFxMatrixContrast : public PostFxMatrix
{
	PostFxMatrixContrast() { m_fContrast = 0.f; }
	virtual Maths::Mat44 Matrix() const;

	F32		m_fContrast;
};

struct PostFxMatrixHue : public PostFxMatrix
{
	PostFxMatrixHue() { m_fMaster = m_fRed = m_fGreen = m_fBlue = 0.f; }
	virtual Maths::Mat44 Matrix() const;

	F32		m_fMaster;
	F32		m_fRed;
	F32		m_fGreen;
	F32		m_fBlue;
};

struct PostFxMatrixSaturation : public PostFxMatrix
{
	PostFxMatrixSaturation() { m_fMaster = m_fRed = m_fGreen = m_fBlue = 0.f; }
	virtual Maths::Mat44 Matrix() const;

	F32		m_fMaster;
	F32		m_fRed;
	F32		m_fGreen;
	F32		m_fBlue;
};

struct PostFxMatrixLightness : public PostFxMatrix
{
	PostFxMatrixLightness()  { m_fMaster = m_fRed = m_fGreen = m_fBlue = 0.f; }
	virtual Maths::Mat44 Matrix() const;

	F32		m_fMaster;
	F32		m_fRed;
	F32		m_fGreen;
	F32		m_fBlue;
};

struct PostFxMatrixAdd : public PostFxMatrix
{
	virtual Maths::Mat44 Matrix() const;

	XtRgb	m_Colour;
};

struct PostFxMatrixMultiply : public PostFxMatrix
{
	virtual Maths::Mat44 Matrix() const;

	XtRgb	m_Colour;
};

struct PostFxMatrixBasis : public PostFxMatrix
{
	virtual Maths::Mat44 Matrix() const;

	XtRgb	m_Red;
	XtRgb	m_Green;
	XtRgb	m_Blue;
};

struct PostFxMatrixSepia : public PostFxMatrix
{
	PostFxMatrixSepia() { m_fSepia = 0.f; }
	virtual Maths::Mat44 Matrix() const;

	F32		m_fSepia;
};

struct PostFxMatrixRotate : public PostFxMatrix
{
	PostFxMatrixRotate() { m_fAmount = 0.f; }
	virtual Maths::Mat44 Matrix() const;

	XtRgb	m_vAxis;
	F32		m_fAmount;
};

struct PostFxMatrixHint : public PostFxMatrix
{
	PostFxMatrixHint() { m_fAmount = 0.f; }
	virtual Maths::Mat44 Matrix() const;

	XtRgb	m_vColour;
	F32		m_fAmount;
};

struct PostFxMatrixCustom : public PostFxMatrix
{
	virtual Maths::Mat44 Matrix() const;

	XtMat44	m_Matrix;
};

}; // namespace Game

#endif // AMAX_GRAPHICS_PostFxMatrix_H
