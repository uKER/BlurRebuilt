#ifndef OBBOBB_INL
#define OBBOBB_INL

#include "gravity/narrowphase/OBBSegment.inl"
#include "gravity/narrowphase/GJK.inl"

namespace Collision
{

//Returns the penetration depth of a point with respect to an axis aligned box and a given unit normal direction.
//The start point must be transformed with respect to the center/orientation of the box	
NOALIAS __forceinline Maths::HF32 PenetrationDepth(const Maths::HVec3 PCREF start, const Maths::HVec3 PCREF d, const Maths::HVec3 PCREF extents)
{
	using namespace Maths;
	//First thing, find the point of exit of the ray from startPoint to direction...
	const HVec3 _max(Maths::Constants<HVec3>::Max());
	const HVec3 _min(Maths::Constants<HVec3>::Zero());

	const HVec3 ood(d.Reciprocal());
	const HVec3 epsilon(Maths::Constants<HVec3>::Epsilon2());
		
	
	const HVec3 absD(Abs(d));
	const Vector3_Bool bSmall(absD < epsilon);
	const Vector3_Bool bOutside(((start < -extents) | (start > extents)));
		
	const HVec3 _t1(((-extents) - start) * ood);
	const HVec3 _t2((extents - start) * ood);
	const Vector3_Bool vb(_t1 < _t2);
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

	const Maths::HF32 pd(Maths::Max(tmax, tmin));
	
	const Scalar_Bool sb(X(bOutside) | Y(bOutside) | Z(bOutside));
	const Scalar_Bool ssb(tmin.LT(tmax) & (!sb));
	//Select the penetration depth..
	return Select(ssb, pd, Maths::Constants<Maths::HF32>::Zero());
}

template <int MaxContacts>
void ProjectBox(const C_OBB& a, const C_OBB& b, C_ContactPatch<MaxContacts>& result, const U32 physicsMaterial, const U32 effectMaterial)
{
	//Project a onto b's local space, then vice-versa
	//First - do from b's local space to a's local space...
	using namespace Maths;

	const HF32 two(Maths::Constants<Maths::HF32>::Two());

	const HVec3 zero(Maths::Constants<Maths::HVec3>::Zero());
	//Put normal into a's local space...
	const HMat33 au(a.Orientation());
	const HMat33 bu(b.Orientation());
	const HMat33 invAu(au.InverseRotation());
	const HMat33 invBu(bu.InverseRotation());
	const HVec3 normA(result.m_Normal * invAu);
	const HVec3 normB((-result.m_Normal) * invBu);
	const HVec3 ac(a.Centre());
	const HVec3 bc(b.Centre());
	const HVec3 ae(a.Extent());
	const HVec3 be(b.Extent());

	//const Maths::HF32 maxPenA(Maths::Abs(normA | a.e) * two);
	//const Maths::HF32 maxPenB(Maths::Abs(normB | b.e) * two);

	const HVec3 _min(-be);
	const HVec3 _max(be);
	const HVec3 transCentre((bc - ac) * invAu);
	const HVec3 transCentre2((ac - bc) * invBu);
	//const Maths::HMat33 rotate(a.u.InverseRotation() * b.u);

	const HVec3 bVerts[8] = 
	{
		((_min * bu + bc) - ac) * invAu,									//000
		((HVec3(_max.X(), _min.Y(), _min.Z()) * bu + bc) - ac) * invAu,		//001
		((HVec3(_min.X(), _max.Y(), _min.Z()) * bu + bc) - ac) * invAu,		//010
		((HVec3(_max.X(), _max.Y(), _min.Z()) * bu + bc) - ac) * invAu,		//011
		((HVec3(_min.X(), _min.Y(), _max.Z()) * bu + bc) - ac) * invAu,		//100
		((HVec3(_max.X(), _min.Y(), _max.Z()) * bu + bc) - ac) * invAu,		//101
		((HVec3(_min.X(), _max.Y(), _max.Z()) * bu + bc) - ac) * invAu,		//110
		((_max * bu + bc) - ac) * invAu,								//111
	};


	U32 numContacts = result.m_NumContactPoints;
	{
		const HVec3 start(bVerts[0]);	
		{
			const HVec3 end(bVerts[1]);
			U32 trueFalse = IntersectLineBox(ae, start, end, normA, result.m_ContactPoints[numContacts].m_ContactPoint, 
				result.m_ContactPoints[numContacts+1].m_ContactPoint,	result.m_ContactPoints[numContacts].m_HashValue, 
				result.m_ContactPoints[numContacts+1].m_HashValue);
			numContacts += (trueFalse << 1);
		}
		if(numContacts >= MaxContacts-1)
			goto TransformPoints1;
		{
			const HVec3 end(bVerts[2]);
			U32 trueFalse = IntersectLineBox(ae, start, end, normA, result.m_ContactPoints[numContacts].m_ContactPoint, 
				result.m_ContactPoints[numContacts+1].m_ContactPoint,	result.m_ContactPoints[numContacts].m_HashValue, 
				result.m_ContactPoints[numContacts+1].m_HashValue);
			numContacts += (trueFalse << 1);
		}
		if(numContacts >= MaxContacts-1)
			goto TransformPoints1;
		{
			const HVec3 end(bVerts[4]);
			U32 trueFalse = IntersectLineBox(ae, start, end, normA, result.m_ContactPoints[numContacts].m_ContactPoint, 
				result.m_ContactPoints[numContacts+1].m_ContactPoint,	result.m_ContactPoints[numContacts].m_HashValue, 
				result.m_ContactPoints[numContacts+1].m_HashValue);
			numContacts += (trueFalse << 1);
		}
	}
	{
		if(numContacts >= MaxContacts-1)
			goto TransformPoints1;
		const HVec3 start(bVerts[1]);	
		{
			const HVec3 end(bVerts[3]);
			U32 trueFalse = IntersectLineBox(ae, start, end, normA, result.m_ContactPoints[numContacts].m_ContactPoint, 
				result.m_ContactPoints[numContacts+1].m_ContactPoint,	result.m_ContactPoints[numContacts].m_HashValue, 
				result.m_ContactPoints[numContacts+1].m_HashValue);
			numContacts += (trueFalse << 1);
		}
		if(numContacts >= MaxContacts-1)
			goto TransformPoints1;
		{
			const HVec3 end(bVerts[5]);
			U32 trueFalse = IntersectLineBox(ae, start, end, normA, result.m_ContactPoints[numContacts].m_ContactPoint, 
				result.m_ContactPoints[numContacts+1].m_ContactPoint,	result.m_ContactPoints[numContacts].m_HashValue, 
				result.m_ContactPoints[numContacts+1].m_HashValue);
			numContacts += (trueFalse << 1);
		}
	}

	{
		if(numContacts >= MaxContacts-1)
			goto TransformPoints1;
		const HVec3 start(bVerts[2]);	
		{
			const HVec3 end(bVerts[3]);
			U32 trueFalse = IntersectLineBox(ae, start, end, normA, result.m_ContactPoints[numContacts].m_ContactPoint, 
				result.m_ContactPoints[numContacts+1].m_ContactPoint,	result.m_ContactPoints[numContacts].m_HashValue, 
				result.m_ContactPoints[numContacts+1].m_HashValue);
			numContacts += (trueFalse << 1);
		}
		if(numContacts >= MaxContacts-1)
			goto TransformPoints1;
		{
			const HVec3 end(bVerts[6]);
			U32 trueFalse = IntersectLineBox(ae, start, end, normA, result.m_ContactPoints[numContacts].m_ContactPoint, 
				result.m_ContactPoints[numContacts+1].m_ContactPoint,	result.m_ContactPoints[numContacts].m_HashValue, 
				result.m_ContactPoints[numContacts+1].m_HashValue);
			numContacts += (trueFalse << 1);
		}
	}

	{
		if(numContacts >= MaxContacts-1)
			goto TransformPoints1;
		const HVec3 start(bVerts[3]);	
		{
			const HVec3 end(bVerts[7]);
			U32 trueFalse = IntersectLineBox(ae, start, end, normA, result.m_ContactPoints[numContacts].m_ContactPoint, 
				result.m_ContactPoints[numContacts+1].m_ContactPoint,	result.m_ContactPoints[numContacts].m_HashValue, 
				result.m_ContactPoints[numContacts+1].m_HashValue);
			numContacts += (trueFalse << 1);
		}
	}

	{
		if(numContacts >= MaxContacts-1)
			goto TransformPoints1;
		const HVec3 start(bVerts[4]);	
		{
			const HVec3 end(bVerts[5]);
			U32 trueFalse = IntersectLineBox(ae, start, end, normA, result.m_ContactPoints[numContacts].m_ContactPoint, 
				result.m_ContactPoints[numContacts+1].m_ContactPoint,	result.m_ContactPoints[numContacts].m_HashValue, 
				result.m_ContactPoints[numContacts+1].m_HashValue);
			numContacts += (trueFalse << 1);
		}
		if(numContacts >= MaxContacts-1)
			goto TransformPoints1;
		{
			const HVec3 end(bVerts[6]);
			U32 trueFalse = IntersectLineBox(ae, start, end, normA, result.m_ContactPoints[numContacts].m_ContactPoint, 
				result.m_ContactPoints[numContacts+1].m_ContactPoint,	result.m_ContactPoints[numContacts].m_HashValue, 
				result.m_ContactPoints[numContacts+1].m_HashValue);
			numContacts += (trueFalse << 1);
		}
	}

	{
		if(numContacts >= MaxContacts-1)
			goto TransformPoints1;
		const HVec3 start(bVerts[5]);	
		{
			const HVec3 end(bVerts[7]);
			U32 trueFalse = IntersectLineBox(ae, start, end, normA, result.m_ContactPoints[numContacts].m_ContactPoint, 
				result.m_ContactPoints[numContacts+1].m_ContactPoint,	result.m_ContactPoints[numContacts].m_HashValue, 
				result.m_ContactPoints[numContacts+1].m_HashValue);
			numContacts += (trueFalse << 1);
		}
	}

	{
		if(numContacts >= MaxContacts-1)
			goto TransformPoints1;
		const HVec3 start(bVerts[6]);	
		{
			const HVec3 end(bVerts[7]);
			U32 trueFalse = IntersectLineBox(ae, start, end, normA, result.m_ContactPoints[numContacts].m_ContactPoint, 
				result.m_ContactPoints[numContacts+1].m_ContactPoint,	result.m_ContactPoints[numContacts].m_HashValue, 
				result.m_ContactPoints[numContacts+1].m_HashValue);
			numContacts += (trueFalse << 1);
		}
	}

TransformPoints1:
	//Loop through and transform all contacts back into world space...
	for(U32 i = result.m_NumContactPoints; i < numContacts; ++i)
	{
		//Contacts in a's local space...
		const Maths::HF32 penDepth1(PenetrationDepth(result.m_ContactPoints[i].m_ContactPoint, normA, ae));
		//Maths::Store(penDepth1, &result.m_ContactPoint[i].m_PenetrationDepth);
		//result.m_ContactPoints[i].m_PenetrationDepth = PenetrationDepth(result.m_ContactPoints[i].m_ContactPoint, normA, a.e);
		result.m_ContactPoints[i].m_ContactPoint = (result.m_ContactPoints[i].m_ContactPoint * au) + ac;
		//Contact point in b's local space...
		const Maths::HVec3 cp2(bu * result.m_ContactPoints[i].m_ContactPoint + bc);
		const Maths::HF32 penDepth2(PenetrationDepth(cp2, normB, be));
		//result.m_ContactPoints[i].m_PenetrationDepth = Maths::Max(result.m_ContactPoints[i].m_PenetrationDepth, penDepth2);
		const Maths::HF32 pen = Maths::Max(penDepth1, penDepth2);
		Maths::Store(pen, &result.m_ContactPoints[i].m_PenetrationDepth);
		result.m_ContactPoints[i].m_PhysicsMaterialIndices = physicsMaterial;
		result.m_ContactPoints[i].m_EffectMaterialIndices = effectMaterial;
	}

	result.m_NumContactPoints = numContacts;
	if(numContacts >= (MaxContacts - 1))
	{
		return;
	}


	const HVec3 min2(-ae);
	const HVec3 max2(ae);

	const HVec3 aVerts[8] = 
	{
		((min2 * au + ac) - bc) * invBu,									//000
		((HVec3(max2.X(), min2.Y(), min2.Z()) * au + ac) - bc) * invBu,		//001
		((HVec3(min2.X(), max2.Y(), min2.Z()) * au + ac) - bc) * invBu,		//010
		((HVec3(max2.X(), max2.Y(), min2.Z()) * au + ac) - bc) * invBu,		//011
		((HVec3(min2.X(), min2.Y(), max2.Z()) * au + ac) - bc) * invBu,		//100
		((HVec3(max2.X(), min2.Y(), max2.Z()) * au + ac) - bc) * invBu,		//101
		((HVec3(min2.X(), max2.Y(), max2.Z()) * au + ac) - bc) * invBu,		//110
		((min2 * au + ac) - bc) * invBu,								//111
	};

	//Now need to iterate through and find intersection points with the AABB

	//U32 trueFalse;
	const U32 startIndex = numContacts;

	{
		const HVec3 start(aVerts[0]);	
		{
			const HVec3 end(aVerts[1]);
			U32 trueFalse = IntersectLineBox(be, start, end, normB, result.m_ContactPoints[numContacts].m_ContactPoint, 
				result.m_ContactPoints[numContacts+1].m_ContactPoint,	result.m_ContactPoints[numContacts].m_HashValue, 
				result.m_ContactPoints[numContacts+1].m_HashValue);
			numContacts += (trueFalse << 1);
		}
		if(numContacts >= MaxContacts-1)
			goto TransformPoints2;
		{
			const HVec3 end(aVerts[2]);
			U32 trueFalse = IntersectLineBox(be, start, end, normB, result.m_ContactPoints[numContacts].m_ContactPoint, 
				result.m_ContactPoints[numContacts+1].m_ContactPoint,	result.m_ContactPoints[numContacts].m_HashValue, 
				result.m_ContactPoints[numContacts+1].m_HashValue);
			numContacts += (trueFalse << 1);
		}
		if(numContacts >= MaxContacts-1)
			goto TransformPoints2;
		{
			const HVec3 end(aVerts[4]);
			U32 trueFalse = IntersectLineBox(be, start, end, normB, result.m_ContactPoints[numContacts].m_ContactPoint, 
				result.m_ContactPoints[numContacts+1].m_ContactPoint,	result.m_ContactPoints[numContacts].m_HashValue, 
				result.m_ContactPoints[numContacts+1].m_HashValue);
			numContacts += (trueFalse << 1);
		}
	}
	{
		if(numContacts >= MaxContacts-1)
			goto TransformPoints2;
		const HVec3 start(aVerts[1]);	
		{
			const HVec3 end(aVerts[3]);
			U32 trueFalse = IntersectLineBox(be, start, end, normB, result.m_ContactPoints[numContacts].m_ContactPoint, 
				result.m_ContactPoints[numContacts+1].m_ContactPoint,	result.m_ContactPoints[numContacts].m_HashValue, 
				result.m_ContactPoints[numContacts+1].m_HashValue);
			numContacts += (trueFalse << 1);
		}
		if(numContacts >= MaxContacts-1)
			goto TransformPoints2;
		{
			const HVec3 end(aVerts[5]);
			U32 trueFalse = IntersectLineBox(be, start, end, normB, result.m_ContactPoints[numContacts].m_ContactPoint, 
				result.m_ContactPoints[numContacts+1].m_ContactPoint,	result.m_ContactPoints[numContacts].m_HashValue, 
				result.m_ContactPoints[numContacts+1].m_HashValue);
			numContacts += (trueFalse << 1);
		}
	}

	{
		if(numContacts >= MaxContacts-1)
			goto TransformPoints2;
		const HVec3 start(aVerts[2]);	
		{
			const HVec3 end(aVerts[3]);
			U32 trueFalse = IntersectLineBox(be, start, end, normB, result.m_ContactPoints[numContacts].m_ContactPoint, 
				result.m_ContactPoints[numContacts+1].m_ContactPoint,	result.m_ContactPoints[numContacts].m_HashValue, 
				result.m_ContactPoints[numContacts+1].m_HashValue);
			numContacts += (trueFalse << 1);
		}
		if(numContacts >= MaxContacts-1)
			goto TransformPoints2;
		{
			const HVec3 end(aVerts[6]);
			U32 trueFalse = IntersectLineBox(be, start, end, normB, result.m_ContactPoints[numContacts].m_ContactPoint, 
				result.m_ContactPoints[numContacts+1].m_ContactPoint,	result.m_ContactPoints[numContacts].m_HashValue, 
				result.m_ContactPoints[numContacts+1].m_HashValue);
			numContacts += (trueFalse << 1);
		}
	}

	{
		if(numContacts >= MaxContacts-1)
			goto TransformPoints2;
		const HVec3 start(aVerts[3]);	
		{
			const HVec3 end(aVerts[7]);
			U32 trueFalse = IntersectLineBox(be, start, end, normB, result.m_ContactPoints[numContacts].m_ContactPoint, 
				result.m_ContactPoints[numContacts+1].m_ContactPoint,	result.m_ContactPoints[numContacts].m_HashValue, 
				result.m_ContactPoints[numContacts+1].m_HashValue);
			numContacts += (trueFalse << 1);
		}
	}

	{
		if(numContacts >= MaxContacts-1)
			goto TransformPoints2;
		const HVec3 start(aVerts[4]);	
		{
			const HVec3 end(aVerts[5]);
			U32 trueFalse = IntersectLineBox(be, start, end, normB, result.m_ContactPoints[numContacts].m_ContactPoint, 
				result.m_ContactPoints[numContacts+1].m_ContactPoint,	result.m_ContactPoints[numContacts].m_HashValue, 
				result.m_ContactPoints[numContacts+1].m_HashValue);
			numContacts += (trueFalse << 1);
		}
		if(numContacts >= MaxContacts-1)
			goto TransformPoints2;
		{
			const HVec3 end(aVerts[6]);
			U32 trueFalse = IntersectLineBox(be, start, end, normB, result.m_ContactPoints[numContacts].m_ContactPoint, 
				result.m_ContactPoints[numContacts+1].m_ContactPoint,	result.m_ContactPoints[numContacts].m_HashValue, 
				result.m_ContactPoints[numContacts+1].m_HashValue);
			numContacts += (trueFalse << 1);
		}
	}

	{
		if(numContacts >= MaxContacts-1)
			goto TransformPoints2;
		const HVec3 start(aVerts[5]);	
		{
			const HVec3 end(aVerts[7]);
			U32 trueFalse = IntersectLineBox(be, start, end, normB, result.m_ContactPoints[numContacts].m_ContactPoint, 
				result.m_ContactPoints[numContacts+1].m_ContactPoint,	result.m_ContactPoints[numContacts].m_HashValue, 
				result.m_ContactPoints[numContacts+1].m_HashValue);
			numContacts += (trueFalse << 1);
		}
	}

	{
		if(numContacts >= MaxContacts-1)
			goto TransformPoints2;
		const HVec3 start(aVerts[6]);	
		{
			const HVec3 end(aVerts[7]);
			U32 trueFalse = IntersectLineBox(be, start, end, normB, result.m_ContactPoints[numContacts].m_ContactPoint, 
				result.m_ContactPoints[numContacts+1].m_ContactPoint,	result.m_ContactPoints[numContacts].m_HashValue, 
				result.m_ContactPoints[numContacts+1].m_HashValue);
			numContacts += (trueFalse << 1);
		}
	}

TransformPoints2:
	//Loop through and transform all contacts back into world space...
	for(U32 i = startIndex; i < numContacts; ++i)
	{
		const Maths::HF32 penDepth1(PenetrationDepth(result.m_ContactPoints[i].m_ContactPoint, normB, be));
		//result.m_ContactPoints[i].m_PenetrationDepth = PenetrationDepth(result.m_ContactPoints[i].m_ContactPoint, normB, b.e);
		result.m_ContactPoints[i].m_ContactPoint = (result.m_ContactPoints[i].m_ContactPoint * bu) + bc;
		const Maths::HVec3 cp2(au * result.m_ContactPoints[i].m_ContactPoint + ac);
		const Maths::HF32 penDepth2(PenetrationDepth(cp2, normA, ae));
		//result.m_ContactPoints[i].m_PenetrationDepth = Maths::Max(result.m_ContactPoints[i].m_PenetrationDepth, penDepth2);
		const Maths::HF32 pen = Maths::Max(penDepth1, penDepth2);
		Maths::Store(pen, & result.m_ContactPoints[i].m_PenetrationDepth);
		result.m_ContactPoints[i].m_PhysicsMaterialIndices = physicsMaterial;
		result.m_ContactPoints[i].m_EffectMaterialIndices = effectMaterial;
	}

	//Now reverse and project a onto b's local space...

	result.m_NumContactPoints = numContacts;
}


template <int MaxContacts>
void ProjectBox33(const C_OBB& a, const C_OBB& b, C_ContactPatch<MaxContacts>& result, const Maths::HMat33 PCREF _b_to_a, const U32 materialIndices)
{
	//Project a onto b's local space, then vice-versa
	//First - do from b's local space to a's local space...

	//Put normal into a's local space...
	using namespace Maths;

	const HMat33& au(a.Orientation());
	const HMat33& bu(b.Orientation());
	const HVec3 ae(a.Extent());
	const HVec3 be(b.Extent());
	const HVec3 ac(a.Centre());
	const HVec3 bc(b.Extent());
	const HVec3 normA(au * (result.m_Normal));
	const HVec3 normB(bu * (-result.m_Normal));

	const HVec3 _min(-be);
	const HVec3 _max(be);
	const HVec3 transCentre(au * (bc - ac));
	//const Maths::HMat33 rotate(a.u.InverseRotation() * b.u);
	const HMat33 rotate(_b_to_a);

	const HVec3 bVerts[8] = 
	{
		_min * rotate + transCentre,									//000
		HVec3(_max.X(), _min.Y(), _min.Z()) * rotate + transCentre,		//001
		HVec3(_min.X(), _max.Y(), _min.Z()) * rotate + transCentre,		//010
		HVec3(_max.X(), _max.Y(), _min.Z()) * rotate + transCentre,		//011
		HVec3(_min.X(), _min.Y(), _max.Z()) * rotate + transCentre,		//100
		HVec3(_max.X(), _min.Y(), _max.Z()) * rotate + transCentre,		//101
		HVec3(_min.X(), _max.Y(), _max.Z()) * rotate + transCentre,		//110
		_max * rotate + transCentre									//111
	};

	//Now need to iterate through and find intersection points with the AABB

	//HVec3 contact1(UNDEFINED), contact2(UNDEFINED);
	//HF32 pen1(UNDEFINED), pen2(UNDEFINED);
	//U32 hashValue1, hashValue2;
	U32 trueFalse;
	U32 numContacts = result.m_NumContactPoints;
	for(U32 i = 0; i < 7; ++i)
	{
		const U32 base = i;
		const U32 right = i | 1;
		const U32 up = i | 2;
		const U32 back = i | 4;
		const HVec3 start(bVerts[base]);

		if(base != right)
		{
			//Line-box check base - right
			const HVec3 end(bVerts[right]);
			trueFalse = IntersectLineBox(ae, start, end, normA, result.m_ContactPoints[numContacts].m_ContactPoint, 
				result.m_ContactPoints[numContacts+1].m_ContactPoint,	result.m_ContactPoints[numContacts].m_HashValue, 
				result.m_ContactPoints[numContacts+1].m_HashValue, result.m_ContactPoints[numContacts].m_PenetrationDepth,
				result.m_ContactPoints[numContacts+1].m_PenetrationDepth);
			result.m_ContactPoints[numContacts].m_MaterialIndices = materialIndices;
			result.m_ContactPoints[numContacts+1].m_MaterialIndices = materialIndices;
			numContacts += (trueFalse << 1);
		}

		if(base != up)
		{
			//Line-box check base - right
			const HVec3 end(bVerts[up]);
			trueFalse = IntersectLineBox(ae, start, end, normA, result.m_ContactPoints[numContacts].m_ContactPoint, 
				result.m_ContactPoints[numContacts+1].m_ContactPoint,	result.m_ContactPoints[numContacts].m_HashValue, 
				result.m_ContactPoints[numContacts+1].m_HashValue, result.m_ContactPoints[numContacts].m_PenetrationDepth,
				result.m_ContactPoints[numContacts+1].m_PenetrationDepth);
			result.m_ContactPoints[numContacts].m_MaterialIndices = materialIndices;
			result.m_ContactPoints[numContacts+1].m_MaterialIndices = materialIndices;
			numContacts += (trueFalse << 1);
		}

		if(base != back)
		{
			//Line-box check base - right
			const HVec3 end(bVerts[back]);
			trueFalse = IntersectLineBox(ae, start, end, normA, result.m_ContactPoints[numContacts].m_ContactPoint, 
				result.m_ContactPoints[numContacts+1].m_ContactPoint,	result.m_ContactPoints[numContacts].m_HashValue, 
				result.m_ContactPoints[numContacts+1].m_HashValue, result.m_ContactPoints[numContacts].m_PenetrationDepth,
				result.m_ContactPoints[numContacts+1].m_PenetrationDepth);
			result.m_ContactPoints[numContacts].m_MaterialIndices = materialIndices;
			result.m_ContactPoints[numContacts+1].m_MaterialIndices = materialIndices;
			numContacts += (trueFalse << 1);
		}
	}

	F32 pen;
	//Loop through and transform all contacts back into world space...
	for(U32 i = result.m_NumContactPoints; i < numContacts; ++i)
	{
		const Maths::HF32 pen1(PenetrationDepth(result.m_ContactPoints[i].m_ContactPoint, normA, ae));
		Maths::Store(pen1, &pen);
		result.m_ContactPoints[i].m_ContactPoint = (result.m_ContactPoints[i].m_ContactPoint * au) + ac;
		result.m_ContactPoints[i].m_PenetrationDepth = Maths::Min(pen, result.m_ContactPoints[i].m_PenetrationDepth);
		//result.m_ContactPoints[i].m_PenetrationDepth = Maths::Min(pen1, result.m_ContactPoints[i].m_PenetrationDepth);
	}

	result.m_NumContactPoints = numContacts;
	if(numContacts >= 12)
	{
		return;
	}

	const HVec3 min2(-ae);
	const HVec3 max2(ae);
	const Maths::HVec3 transCentre2(bu * (ac - bc));
	//const Maths::HMat33 rotate(a.u.InverseRotation() * b.u);

	const HVec3 aVerts[8] = 
	{
		rotate * min2 + transCentre2,									//000
		rotate * HVec3(max2.X(), min2.Y(), min2.Z()) + transCentre2,		//001
		rotate * HVec3(min2.X(), max2.Y(), min2.Z()) + transCentre2,		//010
		rotate * HVec3(max2.X(), max2.Y(), min2.Z()) + transCentre2,		//011
		rotate * HVec3(min2.X(), min2.Y(), max2.Z()) + transCentre2,		//100
		rotate * HVec3(max2.X(), min2.Y(), max2.Z()) + transCentre2,		//101
		rotate * HVec3(min2.X(), max2.Y(), max2.Z()) + transCentre2,		//110
		rotate * max2 + transCentre2									//111
	};

	//Now need to iterate through and find intersection points with the AABB

	for(U32 i = 0; i < 7; ++i)
	{
		const U32 base = i;
		const U32 right = i | 1;
		const U32 up = i | 2;
		const U32 back = i | 4;
		const HVec3 start(aVerts[base]);

		if(base != right)
		{
			//Line-box check base - right
			const HVec3 end(aVerts[right]);
			trueFalse = IntersectLineBox(be, start, end, normB, result.m_ContactPoints[numContacts].m_ContactPoint, 
				result.m_ContactPoints[numContacts+1].m_ContactPoint,	result.m_ContactPoints[numContacts].m_HashValue, 
				result.m_ContactPoints[numContacts+1].m_HashValue, result.m_ContactPoints[numContacts].m_PenetrationDepth,
				result.m_ContactPoints[numContacts+1].m_PenetrationDepth);
			result.m_ContactPoints[numContacts].m_MaterialIndices = materialIndices;
			result.m_ContactPoints[numContacts+1].m_MaterialIndices = materialIndices;
			numContacts += (trueFalse << 1);
		}

		if(base != up)
		{
			//Line-box check base - right
			const HVec3 end(aVerts[up]);
			trueFalse = IntersectLineBox(be, start, end, normB, result.m_ContactPoints[numContacts].m_ContactPoint, 
				result.m_ContactPoints[numContacts+1].m_ContactPoint,	result.m_ContactPoints[numContacts].m_HashValue, 
				result.m_ContactPoints[numContacts+1].m_HashValue, result.m_ContactPoints[numContacts].m_PenetrationDepth,
				result.m_ContactPoints[numContacts+1].m_PenetrationDepth);
			result.m_ContactPoints[numContacts].m_MaterialIndices = materialIndices;
			result.m_ContactPoints[numContacts+1].m_MaterialIndices = materialIndices;
			numContacts += (trueFalse << 1);
		}

		if(base != back)
		{
			//Line-box check base - right
			const HVec3 end(aVerts[back]);
			trueFalse = IntersectLineBox(be, start, end, normB, result.m_ContactPoints[numContacts].m_ContactPoint, 
				result.m_ContactPoints[numContacts+1].m_ContactPoint,	result.m_ContactPoints[numContacts].m_HashValue, 
				result.m_ContactPoints[numContacts+1].m_HashValue, result.m_ContactPoints[numContacts].m_PenetrationDepth,
				result.m_ContactPoints[numContacts+1].m_PenetrationDepth);
			result.m_ContactPoints[numContacts].m_MaterialIndices = materialIndices;
			result.m_ContactPoints[numContacts+1].m_MaterialIndices = materialIndices;
			numContacts += (trueFalse << 1);
		}
	}

	//Loop through and transform all contacts back into world space...
	F32 penF;
	for(U32 i = result.m_NumContactPoints; i < numContacts; ++i)
	{
		const Maths::HF32 pen1(PenetrationDepth(result.m_ContactPoints[i].m_ContactPoint, normB, be));
		Maths::Store(pen1, &penF);
		result.m_ContactPoints[i].m_ContactPoint = (result.m_ContactPoints[i].m_ContactPoint * bu) + bc;
		result.m_ContactPoints[i].m_PenetrationDepth = Maths::Min(penF, result.m_ContactPoints[i].m_PenetrationDepth);
		//result.m_ContactPoints[i].m_PenetrationDepth = Maths::Min(pen1, result.m_ContactPoints[i].m_PenetrationDepth);
	}

	result.m_NumContactPoints = numContacts; 
}

template <int MaxContacts>
bool CollideBoxBox( C_OBB& a, C_OBB& b, C_ContactPatch<MaxContacts>& patch, const U32 physicsMaterial, const U32 effectMaterial,
				   Collision::EPA& epa)
{
	using namespace Maths;

	HVec3 con(UNDEFINED), nrm(UNDEFINED);
	HF32 p(UNDEFINED);
	if(!GJKCPairMargin<C_OBB, C_OBB>(a, b, con, nrm, p, epa))
		return false;

	/*if(!GJKCPair<C_OBB, C_OBB>(a, b, con, nrm, p, epa))
		return false;*/
	

	patch.m_Normal = Maths::C_Normal(nrm, Maths::C_Normal::UNIT_LENGTH_VECTOR);
	patch.m_NumContactPoints = 1;
	patch.m_ContactPoints[0].m_ContactPoint = con;
	//patch.m_ContactPoints[0].m_PenetrationDepth = p;
	Maths::Store(p, &patch.m_ContactPoints[0].m_PenetrationDepth);
	patch.m_ContactPoints[0].m_PhysicsMaterialIndices = physicsMaterial;
	patch.m_ContactPoints[0].m_EffectMaterialIndices = effectMaterial;
	patch.m_ContactPoints[0].m_HashValue = 0;
	//patch.m_NumContactPoints = 0;

	ProjectBox<MaxContacts>(a, b, patch, physicsMaterial, effectMaterial);
	
	
	  
	//assert(patch.m_NumContactPoints > 0);
	return patch.m_NumContactPoints > 0;
}

//template <int MaxPatches, int MaxContacts>
//bool CollideBoxBox2(const C_OBB& a, const C_OBB& b, C_CollisionResult<MaxPatches, MaxContacts>& results, const U32 materialIndices)
//{
//	//const HVec3 epsilon(1e-12f);
//	const HVec3 epsilon(Maths::Constants<HVec3>::Epsilon2());
//	const HMat33 u(a.u);
//	const HVec3 ae(a.e);
//	const HVec3 be(b.e);
//	//Compute the rotation matrix expressing b in a's coordinate
//	const HMat33 _mat(u.InverseRotation()*b.u);
//	/*
//		Compute common subexpressions. Add in an epsilon term to conteract arithmetic errors when
//		two edges are parallel and their cross product is (near) null
//	*/
//	const HMat33 mat(Abs(_mat.AX()) + epsilon, Abs(_mat.AY()) + epsilon, Abs(_mat.AZ()) + epsilon);
//	const HMat33 inverse_mat(mat.InverseRotation());
//
//	//Compute translation vector t
//	const HVec3 _t(b.c-a.c);
//	const HVec3 t(_t*u); 
//	HVec3 dif(UNDEFINED);
//	HVec3 maxDif(UNDEFINED);
//	HMat33 normals(u);
//
//	//Test axes a.u[0], a.u[1], a.u[2]
//	{
//		const HVec3 rb(be*mat);
//		const HVec3 t1(Maths::Abs(t));
//		const HVec3 aerb(ae + rb);
//		//if(!t1.All_LE(aerb))
//		//	return false;
//		const Maths::HVec3 vTmp(aerb - t1);
//		maxDif = Abs(vTmp);
//		//normals = u;
//	}
//
//
//	//Test axes b.u[0], b.u[1], b.u[2]
//	{
//		const HVec3 ra(ae*inverse_mat);
//		const HVec3 t1(t*_mat);
//		const HVec3 t2(Maths::Abs(t1));
//		const HVec3 rarb(ra + be);
//		//if(!t2.All_LE(rarb))
//		//	return false;
//		const Maths::HVec3 vTmp(rarb - t2);
//		const HVec3 dif(Abs(vTmp));
//		const Vector3_Bool vb(dif < maxDif);
//		
//		normals.AX(Select(Maths::Vector3_Bool(Maths::X(vb)), b.u.AX(), normals.AX()));
//		normals.AY(Select(Maths::Vector3_Bool(Maths::Y(vb)), b.u.AY(), normals.AY()));
//		normals.AZ(Select(Maths::Vector3_Bool(Maths::Z(vb)), b.u.AZ(), normals.AZ()));
//		maxDif = Select(vb, dif, maxDif);
//	}
//
//	const HF32 aex(ae.X());
//	const HF32 aey(ae.Y());
//	const HF32 aez(ae.Z());
//
//	const HF32 bex(be.X());
//	const HF32 bey(be.Y());
//	const HF32 bez(be.Z());
//
//	const HVec3 matX(mat.AX());
//	const HVec3 matY(mat.AY());
//	const HVec3 matZ(mat.AZ());
//	
//
//	const HF32 mat00(matX.X());
//	const HF32 mat01(matX.Y());
//	const HF32 mat02(matX.Z());
//
//	const HVec3 _matX(_mat.AX());
//	const HVec3 _matY(_mat.AY());
//	const HVec3 _matZ(_mat.AZ());
//	
//	/*
//		The tests below do not happen very frequecy
//	*/
//	
//	
//	const HVec3 b1(bey, bex, bex);//(1, 0, 0)
//	const HVec3 b2(bez, bez, bey);//(2, 2, 1)
//
//
//	//Batch process a.u[0]X b.u[0], a.u[0] X b.u[1], a.u[0] X b.u[2]
//	{
//	const HVec3 ra((aey*matZ)+(aez*matY));
//
//	const HVec3 temp1(mat02, mat02, mat01);
//	const HVec3 temp2(mat01, mat00, mat00);
//
//	const HVec3 rb((b1 * temp1) + (b2 * temp2));
//
//	const HVec3 absc(Abs((t.Z()*_matY) - (t.Y()*_matZ)));
//	const HVec3 rarb(ra+rb);
//	if(!(absc.All_LT(rarb)))
//		return false;
//	}
//	
//	const HF32 mat10(matY.X());
//	const HF32 mat11(matY.Y());
//	const HF32 mat12(matY.Z());
//	
//	
//	//Batch process a.u[1]X b.u[0], a.u[1] X b.u[1], a.u[1] X b.u[2]
//	{
//	
//	const HVec3 ra((aex*matZ)+(aez*matX));
//
//	const HVec3 temp1(mat12, mat12, mat11);
//	const HVec3 temp2(mat11, mat10, mat10);
//
//	const HVec3 rb((b1 * temp1) + (b2 * temp2));
//	const HVec3 absc(Abs((t.X()*_matZ) - (t.Z()*_matX)));
//	const HVec3 rarb(ra + rb);
//	if(!(absc.All_LT(rarb)))
//		return false;
//	}
//	
//	const HF32 mat20(matZ.X());
//	const HF32 mat21(matZ.Y());
//	const HF32 mat22(matZ.Z());
//	//Batch process a.u[2]X b.u[0], a.u[2] X b.u[1], a.u[2] X b.u[2]
//	{
//	
//	const HVec3 ra((aex*matY)+(aey*matX));
//
//	const HVec3 temp1(mat22, mat22, mat21);
//	const HVec3 temp2(mat21, mat20, mat20);
//
//	const HVec3 rb((b1 * temp1) + (b2 * temp2));
//	const HVec3 absc(Abs((t.Y()*_matX) - (t.X()*_matY)));
//	const HVec3 rarb(ra+rb);
//	if(!(absc.All_LT(rarb)))
//		return false;
//	}
//
//	//OK - got all the vectors. The penetration depth = the min of all 15 floats stored in min1 - min5. 
//	//We also need to know the axis which the smallest penetration sits on. This is as follows
//
//	//Pick minimal axis - get penetration depth and contact normal...
//	results.m_NumContactPatches = 1;
//	C_ContactPatch<MaxContacts> &patch = results.m_ContactPatches[0];
//	const HVec3 mdx(maxDif.X());
//	const HVec3 mdy(maxDif.Y());
//	const HVec3 mdz(maxDif.Z());
//	const Vector3_Bool bv(mdy < mdz);
//	const HVec3 norm(Select(bv, normals.AY(), normals.AZ()));
//	const HVec3 _min(Select(bv, mdy, mdz));
//	const Vector3_Bool vb2(mdx < min);
//	const HF32 dir((norm | _t).Sign());
//	//const HF32 dir2((normals.AX() | _t).Sign());
//	patch.m_Normal = Select(vb2, normals.AX() * -dir, norm * -dir);
//	
//	ProjectBox<MaxContacts>(a, b, patch, _mat, materialIndices);
//	return patch.m_NumContactPoints > 0;
//}

}; //Namespace Collision


#endif
