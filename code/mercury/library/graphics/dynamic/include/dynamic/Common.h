#ifndef RENDER_DYNAMICEFFECT_COMMON_H
#define RENDER_DYNAMICEFFECT_COMMON_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Dynamic/Defines.h"

namespace DynamicEffect
{

	// lookup tables

	struct LookupTable
	{
		Ut::UtVector<Ut::StackVector<MathsStorage::Float4A16, 16> >	table;
		MathsStorage::Float4A16										vSize;
	};

#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR
	FORCE_INLINE XMVECTOR LookupValue(const LookupTable& lookupTable, XMVECTOR vT)
	{
		XMVECTOR vScaled = XMVectorMultiply(vT, XMLoadFloat4A16((XMFLOAT4A16*)&lookupTable.vSize));
		XMVECTOR vTruncate = XMVectorTruncate(vScaled);
		U32 nSegment = (U32)vTruncate.x; // 2x LHS!
		XMVECTOR vInterpolate = XMVectorSubtract(vScaled, vTruncate);

		// this bit is probably slow
		XMFLOAT4A16* pLookup = (XMFLOAT4A16*)&lookupTable.table[nSegment];
		XMVECTOR vLookup0 = XMLoadFloat4A16(pLookup);
		XMVECTOR vLookup1 = XMLoadFloat4A16(++pLookup);

		return XMVectorLerpV(vLookup0, vLookup1, vInterpolate);
	}

	FORCE_INLINE XMVECTOR LookupValue(const LookupTable& lookupTable, XMVECTOR vT, F32 fT)
	{
		fT = fT * lookupTable.vSize.x;
		U32 segment = (U32)fT;

		XMVECTOR vScaled = XMVectorMultiply(vT, XMLoadFloat4A16((XMFLOAT4A16*)&lookupTable.vSize));
		XMVECTOR vTruncate = XMVectorTruncate(vScaled);
		XMVECTOR vInterpolate = XMVectorSubtract(vScaled, vTruncate);

		// this bit is probably slow
		XMFLOAT4A16* pLookup = (XMFLOAT4A16*)&lookupTable.table[(U32)segment];
		XMVECTOR vLookup0 = XMLoadFloat4A16(pLookup);
		XMVECTOR vLookup1 = XMLoadFloat4A16(++pLookup);

		return XMVectorLerpV(vLookup0, vLookup1, vInterpolate);
	}

	FORCE_INLINE XMVECTOR LookupValue(const LookupTable& lookupTable, XMVECTOR vT, U32 segment)
	{
		XMVECTOR vScaled = XMVectorMultiply(vT, XMLoadFloat4A16((XMFLOAT4A16*)&lookupTable.vSize));
		XMVECTOR vTruncate = XMVectorTruncate(vScaled);
		XMVECTOR vInterpolate = XMVectorSubtract(vScaled, vTruncate);

		// this bit is probably slow
		XMFLOAT4A16* pLookup = (XMFLOAT4A16*)&lookupTable.table[segment];
		XMVECTOR vLookup0 = XMLoadFloat4A16(pLookup);
		XMVECTOR vLookup1 = XMLoadFloat4A16(++pLookup);

		return XMVectorLerpV(vLookup0, vLookup1, vInterpolate);
	}

	FORCE_INLINE U32 LookupSegment(const LookupTable& lookupTable, F32 fT)
	{
		fT = fT * lookupTable.vSize.x;
		return (U32)fT;
	}
#endif

//#ifdef RENDER_DYNAMICEFFECT_USE_HVEC
	FORCE_INLINE Maths::HVec4 LookupValue(const LookupTable& lookupTable, const Maths::HF32& vT)
	{
		Maths::HF32 vScaled = vT * lookupTable.vSize.x;
		Maths::HF32 vTruncate = Maths::RoundToIntZero(vScaled);
		U32 nSegment = (U32)vTruncate;
		Maths::HF32 vInterpolate = vScaled - vTruncate;

		// this bit is probably slow
		const MathsStorage::Float4A16* pLookup = &lookupTable.table[nSegment];
		Maths::HVec4 vLookup0(Maths::Load<Maths::HVec4>(pLookup));
		Maths::HVec4 vLookup1(Maths::Load<Maths::HVec4>(++pLookup));

		return vLookup0 + (vLookup1 - vLookup0) * vInterpolate;
	}

	FORCE_INLINE Maths::HVec4 LookupValue(const LookupTable& lookupTable, const Maths::HF32& vT, F32 fT)
	{
		fT = fT * lookupTable.vSize.x;
		U32 segment = (U32)fT;

		Maths::HF32 vScaled = vT * Maths::Load<Maths::HVec4>(&lookupTable.vSize).X();
		Maths::HF32 vTruncate = Maths::RoundToIntZero(vScaled);
		Maths::HF32 vInterpolate = vScaled - vTruncate;

		// this bit is probably slow
		const MathsStorage::Float4A16* pLookup = &lookupTable.table[segment];
		Maths::HVec4 vLookup0(Maths::Load<Maths::HVec4>(pLookup));
		Maths::HVec4 vLookup1(Maths::Load<Maths::HVec4>(++pLookup));

		return vLookup0 + (vLookup1 - vLookup0) * vInterpolate;
	}
//#endif

	FORCE_INLINE F32 LookupValue(const LookupTable& lookupTable, const F32 fT)
	{
		F32 fScaled = fT * lookupTable.vSize.x;
		F32 fTruncate = Maths::RoundToIntZero(fScaled);
		U32 nSegment = (U32)fTruncate;
		F32 fInterpolate = fScaled - fTruncate;

		// this bit is probably slow
		const MathsStorage::Float4A16* pLookup = &lookupTable.table[nSegment];
		const F32 fLookup0 = pLookup[0].x;
		const F32 fLookup1 = pLookup[1].x;

		return fLookup0 + (fLookup1 - fLookup0) * fInterpolate;
	}

	// quaternions

	template <class T>
	NOALIAS INLINE T GetQuaternionFromTo(const Maths::HVec3& vFrom, const Maths::HVec3& vTo)
	{
#ifndef PLATFORM_SPU
		BIZ_STATIC_ASSERT( false );
#endif
	}

	template <>
	NOALIAS INLINE Maths::C_Quaternion GetQuaternionFromTo(const Maths::HVec3& vFrom, const Maths::HVec3& vTo)
	{
		using namespace Maths;
		const HF32 zero( Constants<HF32>::Zero() );
		const HF32 one( Constants<HF32>::One() );
		const HF32 half( Constants<HF32>::Half() );
		HF32 vDot = vFrom | vTo;
		HVec3 vPerp = vFrom ^ vTo;
		HF32 fLength = Clamped(Length(vPerp), zero, one);
		vPerp *= fLength.Reciprocal();

		vDot = Maths::Clamped(vDot, -one, one);

		const Scalar_Bool inputsAreValid = (Maths::Abs(vDot) + Maths::Abs(fLength)).GT(zero);
		vDot = Maths::Select(inputsAreValid, vDot, one);

		HF32 vSinHalfTheta = Maths::Sqrt(Maths::NegMulSub(vDot, half, half));
		HF32 vCosHalfTheta = Maths::Sqrt(Maths::MulAdd(vDot, half, half));

		HVec3 vRotationAxis = Maths::Select(fLength.GT(zero), vPerp, HVec3( c_UnitVectorX ));

		return Maths::C_Quaternion((-vRotationAxis * vSinHalfTheta) || vCosHalfTheta);
	}

	template <>
	NOALIAS INLINE MathsStorage::Float4A16 GetQuaternionFromTo(const Maths::HVec3& vFrom, const Maths::HVec3& vTo)
	{
		using namespace Maths;
		const HF32 zero( Constants<HF32>::Zero() );
		const HF32 one( Constants<HF32>::One() );
		const HF32 half( Constants<HF32>::Half() );
		HF32 vDot = vFrom | vTo;
		HVec3 vPerp = vFrom ^ vTo;
		HF32 fLength = Clamped(Length(vPerp), zero, one);
		vPerp *= fLength.Reciprocal();

		vDot = Maths::Clamped(vDot, -one, one);
		const Scalar_Bool inputsAreValid = (Maths::Abs(vDot) + Maths::Abs(fLength)).GT(zero);
		vDot = Maths::Select(inputsAreValid, vDot, one);

		HF32 vSinHalfTheta = Maths::Sqrt(Maths::NegMulSub(vDot, half, half));
		HF32 vCosHalfTheta = Maths::Sqrt(Maths::MulAdd(vDot, half, half));

		HVec3 vRotationAxis = Maths::Select(fLength.GT(zero), vPerp, HVec3( c_UnitVectorX ));

		Maths::C_Quaternion qRotation((vRotationAxis * vSinHalfTheta) || vCosHalfTheta);
	
		// don't conjugate if we are using XMVECTOR since it is already in correct format
		#ifdef RENDER_DYNAMICEFFECT_USE_HVEC
			qRotation = !qRotation;
		#endif
		
		MathsStorage::Float4A16 rotation;
		Maths::Store(qRotation, &rotation);

		return rotation;
	}

	template <class T>
	NOALIAS INLINE T GetQuaternionTo(const Maths::HVec3& vTo)
	{
#ifndef PLATFORM_SPU
		BIZ_STATIC_ASSERT( false );
#endif
	}

	template <>
	NOALIAS INLINE MathsStorage::Float4A16 GetQuaternionTo(const Maths::HVec3& vTo)
	{
		return GetQuaternionFromTo<MathsStorage::Float4A16>(Maths::Constants<Maths::HVec3>::UnitAxisPositiveY(), vTo);
	}

	template <>
	NOALIAS INLINE Maths::C_Quaternion GetQuaternionTo(const Maths::HVec3& vTo)
	{
		return GetQuaternionFromTo<Maths::C_Quaternion>(Maths::Constants<Maths::HVec3>::UnitAxisPositiveY(), vTo);
	}

	#ifdef RENDER_DYNAMICEFFECT_USE_XMVECTOR

	template <class T>
	NOALIAS INLINE T GetQuaternionFromTo(const XMVECTOR vFrom, const XMVECTOR vTo)
	{
		BIZ_STATIC_ASSERT( false );
	}

	template <>
	NOALIAS INLINE XMVECTOR GetQuaternionFromTo(const XMVECTOR vFrom, const XMVECTOR vTo)
	{
		XMVECTOR vZero(XMVectorZero());
		XMVECTOR vOne(XMVectorSplatOne());

		const XMVECTOR vDefault = { 0.0f, 0.0f, 0.0f, 1.0f };
		const XMVECTOR vUnitX = { 1.0f, 0.0f, 0.0f, 0.0f };
		const XMVECTOR vPi= { 3.14159f, 3.14159f, 3.14159f, 3.14159f }; 

		XMVECTOR vDot = XMVector3Dot(vFrom, vTo);
		XMVECTOR vPerp = XMVector3Cross(vFrom, vTo);
		XMVECTOR vLength = XMVectorClamp(XMVector3Length(vPerp), vZero, vOne);

		if(XMVector4Equal(vLength, vZero))
			return vDefault;
		else
		{
			vPerp = XMVectorMultiply(XMVectorReciprocal(vLength), vPerp);
			XMVECTOR vAngle = XMVectorASin(vLength);

			XMVECTOR vDot_LT_zero = XMVectorLess(vDot, vZero);
			XMVECTOR vLength_EQ_zero = XMVectorEqual(vLength, vZero);

			vPerp = XMVectorSelect(vPerp, vUnitX, XMVectorAndInt(vDot_LT_zero, vLength_EQ_zero));
			vAngle = XMVectorSelect(vAngle, XMVectorSelect(XMVectorSubtract(vPi, vAngle), vPi, vLength_EQ_zero), vDot_LT_zero);

			return XMQuaternionRotationNormal(vPerp, vAngle.x);
		}
	}

	template <class T>
	NOALIAS INLINE T GetQuaternionTo(const XMVECTOR vTo)
	{
		BIZ_STATIC_ASSERT( false );
	}

	template <>
	NOALIAS INLINE XMVECTOR GetQuaternionTo(const XMVECTOR vTo)
	{
		const XMVECTOR vFrom = { 0.0f, 1.0f, 0.0f, 0.0f };
		return GetQuaternionFromTo<XMVECTOR>(vFrom, vTo);
	}

	#endif

}; // namespace DynamicEffect

#endif // RENDER_DYNAMICEFFECT_COMMON_H
