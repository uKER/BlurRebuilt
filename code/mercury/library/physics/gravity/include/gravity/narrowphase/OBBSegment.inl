#ifndef OBBSEGMENT_INL
#define OBBSEGMENT_INL

#include "Core/Maths/Hvec/VectorInt.h"
#include "Core/Maths/Hvec/VectorInt.inl"

namespace Collision
{

	//const U32 _frontIndices[] = {1, 2, 3};
	//const U32 _backIndices[] = {4, 5, 6};

	const U32* const __restrict FrontIndices();
	const U32* const __restrict BackIndices();

	//using namespace Maths;
	NOALIAS __forceinline U32 IntersectLineBox(const Maths::HVec3 PCREF extents, const Maths::HVec3 PCREF start, const Maths::HVec3 PCREF end, const Maths::HVec3 PCREF localNormal, Maths::HVec3& contactPoint1, Maths::HVec3& contactPoint2, U32& faceIndex1, U32& faceIndex2)
	{
		using namespace Maths;
		const U32* const __restrict _frontIndices = FrontIndices();						//-x, -y, -z
		const U32* const __restrict _backIndices = BackIndices();						//x, y, z
		const HVec3 d(end - start);
		const HVec3 ood(d.Reciprocal());
		const HVec3 epsilon(Maths::Constants<HVec3>::Epsilon());
		const HVec3 _max(Maths::Constants<HVec3>::One());
		const HVec3 _min(Maths::Constants<HVec3>::Zero());
		
		//Check to see if any of the extents are outside of the box...
		const Vector3_U32 origFrontIndices = Load<Vector3_U32>(_frontIndices);
		const Vector3_U32 origBackIndices = Load<Vector3_U32>(_backIndices);
		const HVec3 absD(Abs(d));
		const Vector3_Bool bSmall(absD < epsilon);
		const Vector3_Bool bOutside((bSmall) & ((start < -extents) | (start > extents)));
		
		const HVec3 _t1(((-extents) - start) * ood);
		const HVec3 _t2((extents - start) * ood);
		const Vector3_Bool vb(_t1 < _t2);
		const Vector3_U32 frontIndices(Select(vb, origFrontIndices, origBackIndices));
		const Vector3_U32 backIndices(Select(vb, origBackIndices, origFrontIndices));
		const HVec3 t11(Select(vb, _t1, _t2) + epsilon);
		const HVec3 t21(Select(vb, _t2, _t1) - epsilon);

		const HVec3 t1(Select(bSmall, _min, Max(t11, _min)));
		const HVec3 t2(Select(bSmall, _max, Min(t21, _max)));

		const HF32 t1x(t1.X());
		const HF32 t1y(t1.Y());
		const HF32 t1z(t1.Z());
		const HF32 t2x(t2.X());
		const HF32 t2y(t2.Y());
		const HF32 t2z(t2.Z());


		const Scalar_Bool sb1(t1y.GT(t1z));
		const HF32 tmin_tmp(Select(sb1, t1y, t1z));
		const Scalar_Bool sb2(tmin_tmp.GT(t1x));
		const HF32 tmin(Select(sb2, tmin_tmp, t1x));

		const Scalar_Bool sb3(t2y.LT(t2z));
		const HF32 tmax_tmp=Select(sb3, t2y, t2z);
		const Scalar_Bool sb4(tmax_tmp.LT(t2x));
		const HF32 tmax=Select(sb4, tmax_tmp, t2x);

		//Pick the index for the face value...probably a slightly faster way, but for now, just pick the 
		//index defined by t1?
		const Scalar_U32 index1 = Select(sb2, Select(sb1, Y(frontIndices), Z(frontIndices)), X(frontIndices));
		const Scalar_U32 index2 = Select(sb4, Select(sb3, Y(backIndices), Z(backIndices)), X(backIndices));

		//faceIndex = Maths::X(index);
		Store(index1, &faceIndex1);
		Store(index2, &faceIndex2);
		
		//TODO - penetration depth!!!!!!! A bit ropey at the moment!!!

		contactPoint1 = start + d * tmin;
		contactPoint2 = start + d * tmax;
		//const HF32 penDepth(Select(sb2, Select(sb1, vec.Y(), vec.Z()), vec.X()));

		//Work out what the contact point is...
		
		const Scalar_Bool sb(X(bOutside) | Y(bOutside) | Z(bOutside));
		return ((tmin.LT(tmax)) & (!sb)) != 0;
	}

	//using namespace Maths;
	NOALIAS __forceinline U32 IntersectLineBox(const Maths::HVec3 PCREF extents, const Maths::HVec3 PCREF start, const Maths::HVec3 PCREF end, const Maths::HVec3 PCREF localNormal, Maths::HVec3& contactPoint1, Maths::HVec3& contactPoint2, U32& faceIndex1, U32& faceIndex2, Maths::HF32& penetrationDepth1, Maths::HF32& penetrationDepth2)
	{
		using namespace Maths;
		const U32* const __restrict _frontIndices = FrontIndices();						//-x, -y, -z
		const U32* const __restrict _backIndices = BackIndices();						//x, y, z
		const HVec3 d(end - start);
		const HVec3 ood(d.Reciprocal());
		const HVec3 epsilon(Maths::Constants<HVec3>::Epsilon());
		const HVec3 _max(Maths::Constants<HVec3>::One());
		const HVec3 _min(Maths::Constants<HVec3>::Zero());
		
		//Check to see if any of the extents are outside of the box...
		const Vector3_U32 origFrontIndices = Load<Vector3_U32>(_frontIndices);
		const Vector3_U32 origBackIndices = Load<Vector3_U32>(_backIndices);
		const HVec3 absD(Abs(d));
		const Vector3_Bool bSmall(absD < epsilon);
		const Vector3_Bool bOutside((bSmall) & ((start < -extents) | (start > extents)));
		
		const HVec3 _t1(((-extents) - start) * ood);
		const HVec3 _t2((extents - start) * ood);
		const Vector3_Bool vb(_t1 < _t2);
		const Vector3_U32 frontIndices(Select(vb, origFrontIndices, origBackIndices));
		const Vector3_U32 backIndices(Select(vb, origBackIndices, origFrontIndices));
		const HVec3 t11(Select(vb, _t1, _t2));
		const HVec3 t21(Select(vb, _t2, _t1));

		const HVec3 t1(Select(bSmall, _min, Max(t11, _min)));
		const HVec3 t2(Select(bSmall, _max, Min(t21, _max)));

		const HF32 t1x(t1.X());
		const HF32 t1y(t1.Y());
		const HF32 t1z(t1.Z());
		const HF32 t2x(t2.X());
		const HF32 t2y(t2.Y());
		const HF32 t2z(t2.Z());


		const Scalar_Bool sb1(t1y.GT(t1z));
		const HF32 tmin_tmp(Select(sb1, t1y, t1z));
		const Scalar_Bool sb2(tmin_tmp.GT(t1x));
		const HF32 tmin(Select(sb2, tmin_tmp, t1x));

		const Scalar_Bool sb3(t2y.LT(t2z));
		const HF32 tmax_tmp=Select(sb3, t2y, t2z);
		const Scalar_Bool sb4(tmax_tmp.LT(t2x));
		const HF32 tmax=Select(sb4, tmax_tmp, t2x);

		//Pick the index for the face value...probably a slightly faster way, but for now, just pick the 
		//index defined by t1?
		const Scalar_U32 index1 = Select(sb2, Select(sb1, Y(frontIndices), Z(frontIndices)), X(frontIndices));
		const Scalar_U32 index2 = Select(sb4, Select(sb3, Y(backIndices), Z(backIndices)), X(backIndices));

		//faceIndex = Maths::X(index);
		Store(index1, &faceIndex1);
		Store(index2, &faceIndex2);
		
		//TODO - penetration depth!!!!!!! A bit ropey at the moment!!!

		const HF32 outVal(tmax);
		const HF32 penRatio(outVal - tmin);
		const HVec3 vec(d * penRatio);
		const HF32 penDepth(Select(sb2, Select(sb1, vec.Y(), vec.Z()), vec.X()));
		penetrationDepth1 = penDepth;
		penetrationDepth2 = penDepth;

		//Work out what the contact point is...
		contactPoint1 = start + d * tmin;
		contactPoint2 = start + d * tmax;
		const Scalar_Bool sb(X(bOutside) | Y(bOutside) | Z(bOutside));
		return ((tmin.LT(tmax)) & (!sb)) != 0;
	}

};

#endif
