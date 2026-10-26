#include "stdafx.h"
#include <Graphics/PostFxMatrix.h>

#include <Core/Maths/HVec/New/IncludeMinimal.h>
#include <Core/Maths/HVec/New/IncludeMinimal.inl>

XT_TYPE(Game::PostFxMatrix, "PostFxMatrix")
	XT_FIELD(m_pOut, "out")
	XT_FIELD(m_bEnabled, "enabled")
XT_END_TYPE

XT_TYPE(Game::PostFxMatrixRoot, "PostFxMatrixRoot")
	XT_BASE(Game::PostFxMatrix)
XT_END_TYPE

XT_TYPE(Game::PostFxMatrixBrightness, "PostFxMatrixBrightness")
	XT_BASE(Game::PostFxMatrix)
	XT_FIELD(m_fBrightness, "brightness")
XT_END_TYPE

XT_TYPE(Game::PostFxMatrixContrast, "PostFxMatrixContrast")
	XT_BASE(Game::PostFxMatrix)
	XT_FIELD(m_fContrast, "contrast")
XT_END_TYPE

XT_TYPE(Game::PostFxMatrixHue, "PostFxMatrixHue")
	XT_BASE(Game::PostFxMatrix)
	XT_FIELD(m_fMaster, "master")
	XT_FIELD(m_fRed, "red")
	XT_FIELD(m_fGreen, "green")
	XT_FIELD(m_fBlue, "blue")
XT_END_TYPE

XT_TYPE(Game::PostFxMatrixSaturation, "PostFxMatrixSaturation")
	XT_BASE(Game::PostFxMatrix)
	XT_FIELD(m_fMaster, "master")
	XT_FIELD(m_fRed, "red")
	XT_FIELD(m_fGreen, "green")
	XT_FIELD(m_fBlue, "blue")
XT_END_TYPE

XT_TYPE(Game::PostFxMatrixLightness, "PostFxMatrixLightness")
	XT_BASE(Game::PostFxMatrix)
	XT_FIELD(m_fMaster, "master")
	XT_FIELD(m_fRed, "red")
	XT_FIELD(m_fGreen, "green")
	XT_FIELD(m_fBlue, "blue")
XT_END_TYPE

XT_TYPE(Game::PostFxMatrixAdd, "PostFxMatrixAdd")
	XT_BASE(Game::PostFxMatrix)
	XT_FIELD(m_Colour, "colour")
XT_END_TYPE

XT_TYPE(Game::PostFxMatrixMultiply, "PostFxMatrixMultiply")
	XT_BASE(Game::PostFxMatrix)
	XT_FIELD(m_Colour, "colour")
XT_END_TYPE

XT_TYPE(Game::PostFxMatrixBasis, "PostFxMatrixBasis")
	XT_BASE(Game::PostFxMatrix)
	XT_FIELD(m_Red, "red")
	XT_FIELD(m_Green, "green")
	XT_FIELD(m_Blue, "blue")
XT_END_TYPE

XT_TYPE(Game::PostFxMatrixSepia, "PostFxMatrixSepia")
	XT_BASE(Game::PostFxMatrix)
	XT_FIELD(m_fSepia, "sepia")
XT_END_TYPE

XT_TYPE(Game::PostFxMatrixRotate, "PostFxMatrixRotate")
	XT_BASE(Game::PostFxMatrix)
	XT_FIELD(m_vAxis, "axis")
	XT_FIELD(m_fAmount, "amount")
XT_END_TYPE

XT_TYPE(Game::PostFxMatrixHint, "PostFxMatrixHint")
	XT_BASE(Game::PostFxMatrix)
	XT_FIELD(m_vColour, "colour")
	XT_FIELD(m_fAmount, "amount")
XT_END_TYPE

XT_TYPE(Game::PostFxMatrixCustom, "PostFxMatrixCustom")
	XT_BASE(Game::PostFxMatrix)
	XT_FIELD(m_Matrix, "matrix")
XT_END_TYPE

namespace Game
{

Maths::Mat44 PostFxMatrix::Matrix() const
{
	return Maths::Const<Maths::Mat44>::Identity();
}

Maths::Mat44 PostFxMatrixBrightness::Matrix() const
{
	MathsStorage::MatrixRow44 m =
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		m_fBrightness, m_fBrightness, m_fBrightness, 1.0f,
	};
	return Maths::Load<Maths::Mat44>(&m);
}

Maths::Mat44 PostFxMatrixContrast::Matrix() const
{
	const F32 c = m_fContrast + 1.0f; // m_fContrast is in range -1 to 1
	MathsStorage::MatrixRow44 m =
	{
		c, 0.0f, 0.0f, 0.0f,
		0.0f, c, 0.0f, 0.0f,
		0.0f, 0.0f, c, 0.0f,
		0.5f - 0.5f * c, 0.5f - 0.5f * c, 0.5f - 0.5f * c, 1.0f,
	};
	return Maths::Load<Maths::Mat44>(&m);
}

Maths::Mat44 PostFxMatrixHue::Matrix() const
{
	const F32 fSqrt2 = sqrt(2.0f);
	const F32 fSqrt3 = sqrt(3.0f);
	const F32 fSqrt6 = fSqrt2 * fSqrt3;
	MathsStorage::MatrixRow44 fRotate =
	{
		fSqrt2 / fSqrt3, 0.0f, 1.0f / fSqrt3, 0.0f,
		-1.0f / fSqrt6, 1.0f / fSqrt2, 1.0f / fSqrt3, 0.0f,
		-1.0f / fSqrt6, -1.0f / fSqrt2, 1.0f / fSqrt3, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};
	Maths::Mat44 mRotate = Maths::Load<Maths::Mat44>(&fRotate);	

	const F32 fLR = 0.2125f;
	const F32 fLG = 0.7154f;
	const F32 fLB = 0.0721f;
	const F32 fLRx = fLR * (fSqrt2 / fSqrt3) + fLG * (-1.0f / fSqrt6) + fLB * (-1.0f / fSqrt6);
	const F32 fLGx = fLR * (0.0f) + fLG * (1.0f / fSqrt2) + fLB * (-1.0f / fSqrt2);
	const F32 fLBx = fLR * (1.0f / fSqrt3) + fLG * (1.0f / fSqrt3) + fLB * (1.0f / fSqrt3);
	MathsStorage::MatrixRow44 fShear =
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		-fLRx / fLBx, -fLGx / fLBx, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};
	Maths::Mat44 mShear = Maths::Load<Maths::Mat44>(&fShear);

	const F32 fRedAngle = (m_fMaster + m_fRed) * (Maths::Pi / 180.0f);
	const F32 fRedSin = sinf(fRedAngle);
	const F32 fRedCos = cosf(fRedAngle);
	MathsStorage::MatrixRow44 fRed =
	{
		fRedCos, fRedSin, 0.0f, 0.0f,
		-fRedSin, fRedCos, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};
	Maths::Mat44 mRed = Maths::Load<Maths::Mat44>(&fRed);

	const F32 fGreenAngle = (m_fMaster + m_fGreen) * (Maths::Pi / 180.0f);
	const F32 fGreenSin = sinf(fGreenAngle);
	const F32 fGreenCos = cosf(fGreenAngle);
	MathsStorage::MatrixRow44 fGreen =
	{
		fGreenCos, fGreenSin, 0.0f, 0.0f,
		-fGreenSin, fGreenCos, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};
	Maths::Mat44 mGreen = Maths::Load<Maths::Mat44>(&fGreen);

	const F32 fBlueAngle = (m_fMaster + m_fBlue) * (Maths::Pi / 180.0f);
	const F32 fBlueSin = sinf(fBlueAngle);
	const F32 fBlueCos = cosf(fBlueAngle);
	MathsStorage::MatrixRow44 fBlue =
	{
		fBlueCos, fBlueSin, 0.0f, 0.0f,
		-fBlueSin, fBlueCos, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};
	Maths::Mat44 mBlue = Maths::Load<Maths::Mat44>(&fBlue);

//	Maths::Mat44 mFinalRed = mRotate * mShear * mRed * Maths::Inverse(mShear) * Maths::Inverse(mRotate);
//	Maths::Mat44 mFinalGreen = mRotate * mShear * mBlue * Maths::Inverse(mShear) * Maths::Inverse(mRotate);
//	Maths::Mat44 mFinalBlue = mRotate * mShear * mGreen * Maths::Inverse(mShear) * Maths::Inverse(mRotate);
	Maths::Mat44 mFinalRed = mRotate * mRed * Maths::Inverse(mRotate);
	Maths::Mat44 mFinalGreen = mRotate * mBlue * Maths::Inverse(mRotate);
	Maths::Mat44 mFinalBlue = mRotate * mGreen * Maths::Inverse(mRotate);
	
	return Maths::MakeMat44(Maths::RowX(mFinalRed), Maths::RowY(mFinalGreen), Maths::RowZ(mFinalBlue), Maths::RowW(mFinalRed));
}

Maths::Mat44 PostFxMatrixSaturation::Matrix() const
{
	const F32 fR = Maths::Clamp(m_fMaster + m_fRed, -1.0f, 1.0f) + 1.0f; 
	const F32 fG = Maths::Clamp(m_fMaster + m_fGreen, -1.0f, 1.0f) + 1.0f; 
	const F32 fB = Maths::Clamp(m_fMaster + m_fBlue, -1.0f, 1.0f) + 1.0f; 

	const F32 fLR = 0.2125f;
	const F32 fLG = 0.7154f;
	const F32 fLB = 0.0721f;

	MathsStorage::MatrixRow44 m =
	{
		(1.0f - fR) * fLR + fR, (1.0f - fR) * fLR, (1.0f - fR) * fLR, 0.0f,
		(1.0f - fG) * fLG, (1.0f - fG) * fLG + fG, (1.0f - fG) * fLG, 0.0f,
		(1.0f - fB) * fLB, (1.0f - fB) * fLB, (1.0f - fB) * fLB + fB, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	return Maths::Load<Maths::Mat44>(&m);
}

Maths::Mat44 PostFxMatrixLightness::Matrix() const
{
	const F32 fRedScale			= Maths::Clamp(m_fMaster + m_fRed, -1.0f, 1.0f) + 1.0f;
	const F32 fGreenScale		= Maths::Clamp(m_fMaster + m_fGreen, -1.0f, 1.0f) + 1.0f;
	const F32 fBlueScale		= Maths::Clamp(m_fMaster + m_fBlue, -1.0f, 1.0f) + 1.0f;
	Maths::Mat33 mYUV			= Maths::MakeMat33(0.299f, -0.14713f, 0.615f, 0.587f, -0.28886f, -0.51499f, 0.114f, 0.436f, -0.10001f);
	Maths::Mat33 mRedScale		= Maths::MakeMat33(fRedScale, 0.0f, 0.0f, 0.0f, fRedScale, 0.0f, 0.0f, 0.0f, fRedScale);
	Maths::Mat33 mGreenScale	= Maths::MakeMat33(fGreenScale, 0.0f, 0.0f, 0.0f, fGreenScale, 0.0f, 0.0f, 0.0f, fGreenScale);
	Maths::Mat33 mBlueScale		= Maths::MakeMat33(fBlueScale, 0.0f, 0.0f, 0.0f, fBlueScale, 0.0f, 0.0f, 0.0f, fBlueScale);

	Maths::Mat33 mRed			= mYUV * mRedScale * Maths::Inverse(mYUV);
	Maths::Mat33 mGreen			= mYUV * mGreenScale * Maths::Inverse(mYUV);
	Maths::Mat33 mBlue			= mYUV * mBlueScale * Maths::Inverse(mYUV);
	Maths::Mat33 mFinal			= Maths::MakeMat33(Maths::RowX(mRed), Maths::RowY(mGreen), Maths::RowZ(mBlue));

	return Maths::MakeMat44(mFinal);
}

Maths::Mat44 PostFxMatrixAdd::Matrix() const
{
	MathsStorage::MatrixRow44 m =
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		m_Colour.r, m_Colour.g, m_Colour.b, 1.0f,
	};
	return Maths::Load<Maths::Mat44>(&m);
}

Maths::Mat44 PostFxMatrixMultiply::Matrix() const
{
	MathsStorage::MatrixRow44 m =
	{
		m_Colour.r, 0.0f, 0.0f, 0.0f,
		0.0f, m_Colour.g, 0.0f, 0.0f,
		0.0f, 0.0f, m_Colour.b, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};
	return Maths::Load<Maths::Mat44>(&m);
}

Maths::Mat44 PostFxMatrixBasis::Matrix() const
{
	MathsStorage::MatrixRow44 m =
	{
		m_Red.r, m_Red.g, m_Red.b, 0.0f,
		m_Green.r, m_Green.g, m_Green.b, 0.0f,
		m_Blue.r, m_Blue.g, m_Blue.b, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};
	return Maths::Load<Maths::Mat44>(&m);
}

Maths::Mat44 PostFxMatrixSepia::Matrix() const
{
	MathsStorage::MatrixRow44 m =
	{
		0.393f * m_fSepia + 1.0f - m_fSepia, 0.349f * m_fSepia, 0.272f * m_fSepia, 0.0f,
		0.769f * m_fSepia, 0.686f * m_fSepia + 1.0f - m_fSepia, 0.534f * m_fSepia, 0.0f,
		0.189f * m_fSepia, 0.168f * m_fSepia, 0.131f * m_fSepia + 1.0f - m_fSepia, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f,
	};
	return Maths::Load<Maths::Mat44>(&m);
}

Maths::Mat44 PostFxMatrixRotate::Matrix() const
{
	Maths::Normal3	vAxis	= Maths::Normalise(Maths::MakeVector3(m_vAxis.r, m_vAxis.g, m_vAxis.b));
	Maths::VF32		vAngle	= Maths::MakeVF32(m_fAmount * Maths::Pi / 180.0f);
	Maths::Mat33	mRotate	= Maths::MakeMat33_AxisAngle(vAxis, vAngle);
	return Maths::MakeMat44(mRotate);
}

Maths::Mat44 PostFxMatrixHint::Matrix() const
{
	Maths::Vector3	vRed	= Maths::MakeVector3(1.0f, 0.0f, 0.0f);
	Maths::Vector3	vGreen	= Maths::MakeVector3(0.0f, 1.0f, 0.0f);
	Maths::Vector3	vBlue	= Maths::MakeVector3(0.0f, 0.0f, 1.0f);
	Maths::Vector3	vColour	= Maths::MakeVector3(m_vColour.r / 3.0f, m_vColour.g / 3.0f, m_vColour.b / 3.0f);
	Maths::VF32		vBlend	= Maths::MakeVF32(m_fAmount);

	vRed	= Maths::Lerp(vRed, vColour, vBlend);
	vGreen	= Maths::Lerp(vGreen, vColour, vBlend);
	vBlue	= Maths::Lerp(vBlue, vColour, vBlend);

	Maths::Mat33	mFinal	= Maths::MakeMat33(vRed, vGreen, vBlue);

	return Maths::MakeMat44(mFinal);
}

Maths::Mat44 PostFxMatrixCustom::Matrix() const
{
	MathsStorage::MatrixRow44 m =
	{
		m_Matrix.ax.vx, m_Matrix.ax.vy, m_Matrix.ax.vz, m_Matrix.ax.vw,
		m_Matrix.ay.vx, m_Matrix.ay.vy, m_Matrix.ay.vz, m_Matrix.ay.vw,
		m_Matrix.az.vx, m_Matrix.az.vy, m_Matrix.az.vz, m_Matrix.az.vw,
		m_Matrix.aw.vx, m_Matrix.aw.vy, m_Matrix.aw.vz, m_Matrix.aw.vw,
	};
	return Maths::Load<Maths::Mat44>(&m);
}

}; // namespace Game
