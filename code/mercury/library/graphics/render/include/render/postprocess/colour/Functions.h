#ifndef RENDER_POSTPROCESS_COLOUR_FUNCTIONS_H
#define RENDER_POSTPROCESS_COLOUR_FUNCTIONS_H

#if _MSC_VER > 1000
#pragma once
#endif

namespace PostProcess
{

INLINE Maths::Vec4F32 Greyscale(const Maths::Vec4F32& vColour)
{
	const F32 fScales[4] = { 0.30f, 0.59f, 0.11f, 0.00f };
	
	const Maths::Vec4F32	vScales		= Maths::Load<Maths::Vec4F32>(fScales);
	const Maths::VF32		fGreyscale	= Maths::Dot3(vScales, vColour);

	return Maths::MakeVec4F32(fGreyscale);
}

INLINE Maths::Vec4F32 AdjustSaturation(const Maths::Vec4F32& vColour, const Maths::Vec4F32& vAmount)
{
	const F32 fScales[4] = { 0.2125f, 0.7154f, 0.0721f, 0.000f };

	const Maths::Vec4F32 vScales	= Maths::Load<Maths::Vec4F32>(fScales);
	const Maths::Vec4F32 vLuminance	= Maths::MakeVec4F32(Maths::Dot3(vScales, vColour));
	
	return (vColour - vLuminance) * vAmount + vLuminance;
}

INLINE Maths::Vec4F32 AdjustSaturation(const Maths::Vec4F32& vColour, const Maths::VF32& fAmount)
{
	const F32 fScales[4] = { 0.2125f, 0.7154f, 0.0721f, 0.000f };

	const Maths::Vec4F32 vScales	= Maths::Load<Maths::Vec4F32>(fScales);
	const Maths::Vec4F32 vLuminance	= Maths::MakeVec4F32(Maths::Dot3(vScales, vColour));
	
	return (vColour - vLuminance) * fAmount + vLuminance;
}

INLINE Maths::Vec4F32 ApplySepia(const Maths::Vec4F32& vColour, const Maths::VF32& fAmount)
{
	const MathsStorage::MatrixRow33 mValues = { 0.393f, 0.349f, 0.272f, 0.769f, 0.686f, 0.534f, 0.189f, 0.168f, 0.131f };
	const Maths::Mat33 mSepia	= Maths::Load<Maths::Mat33>(&mValues);
	const Maths::Vec4F32 vSepia	= vColour * mSepia;
	
	return Maths::Lerp(vColour, vSepia, fAmount);
}

INLINE Maths::Vec4F32 AdjustBrightness(const Maths::Vec4F32& vColour, const Maths::VF32& fAmount)
{
	const Maths::Vec4F32	vZero		= Maths::Const<Maths::Vec4F32>::Zero();
	const Maths::Vec4F32	vOne		= Maths::Const<Maths::Vec4F32>::One();

	return Maths::Clamp(vColour + Maths::MakeVec4F32(fAmount), vZero, vOne);
}

INLINE Maths::Vec4F32 AdjustContrast(const Maths::Vec4F32& vColour, const Maths::VF32& fAmount)
{
	Maths::Vec4F32	vHalf		= Maths::Const<Maths::Vec4F32>::Half();
	Maths::Vec4F32	vZero		= Maths::Const<Maths::Vec4F32>::Zero();
	Maths::Vec4F32	vOne		= Maths::Const<Maths::Vec4F32>::One();
	Maths::Vec4F32	vAdjusted	= vColour - vHalf;
					vAdjusted	*= fAmount;
					vAdjusted	+= vHalf;
	return Maths::Clamp(vAdjusted, vZero, vOne);
}

INLINE Maths::Vec4F32 ApplyGamma(const Maths::Vec4F32& vColour, const Maths::VF32& fGamma)
{
	return Maths::Pow(vColour, Maths::MakeVec4F32(fGamma));
}

INLINE Maths::Vec4F32 ApplyModulateAdd(const Maths::Vec4F32& vColour, const Maths::Vec4F32& vModulate, const Maths::Vec4F32& vAdd)
{
	return Maths::MulAdd(vColour, vModulate, vAdd);
}

}; // namespace PostProcess

#endif // RENDER_POSTPROCESS_COLOUR_FUNCTIONS_H
