template<typename VECTYPE>
F32 InterpolateKeyData(
	const VECTYPE& keyPosA, 
	F32 uA,
	const VECTYPE& keyPosB, 
	F32 uB,
	const VECTYPE& pos)
{
	VECTYPE keyVect = keyPosB - keyPosA;
	F32 sectLength = keyVect.Length();
	VECTYPE AtoPos = pos - keyPosA;
	F32 posLength = AtoPos.Length();
	AtoPos.Normalise();
	keyVect.Normalise();
	F32 dot =  AtoPos| keyVect;

	// if pos is behind then closest pos is keyA
	dot = Maths::Max(dot, 0.0f);
	F32 scaleLength = posLength * dot;

	F32 sectPos = uB - uA;

	return (sectPos * (scaleLength/sectLength));
}

template<typename VECTYPE>
VECTYPE InterpolatePosLinear(
	const VECTYPE& keyPosA, 
	F32 uA,
	const VECTYPE& keyPosB, 
	F32 uB,
	const VECTYPE& pos, 
	F32& splinePos)
{
	VECTYPE keyVect = keyPosB - keyPosA;
	F32 sectLength = keyVect.Length();
	VECTYPE AtoPos = pos - keyPosA;
	F32 posLength = AtoPos.Length();
	AtoPos.Normalise();
	keyVect.Normalise();
	F32 dot =  AtoPos| keyVect;

	// if pos is behind then closest pos is keyA
	dot = Maths::Max(dot, 0.0f);
	F32 scaleLength = posLength * dot;

	F32 AtoBposition = uB - uA;
	splinePos = uA + ((scaleLength/sectLength) * AtoBposition);
	return (keyPosA + (scaleLength * keyVect));
}



template<typename CURVE>
F32 FindNearestPoint(
	const CURVE& curve,
	const C_Reparameterise& reparam,
	const typename CURVE::VecType& pos, 
	typename CURVE::VecType& nearest) 
{	
	F32 nearestDistance2 = FLT_MAX;
	U32 nearestSpan = 0;
	F32 arcLength = 0;
	for (U32 i = 1; i < reparam.GetNumPoints(); ++i)
	{
		typename CURVE::VecType a = ReparamHelpers::GetPointValue(curve, reparam, i-1);
		typename CURVE::VecType b = ReparamHelpers::GetPointValue(curve, reparam, i);
		Maths::HF32 t;
		typename CURVE::VecType nearestCandidate;
		Collision::ClosestPtPointSegment(pos, a, b, nearestCandidate, t);
	
		F32 distance2 = (F32)Maths::DistanceSquared(nearestCandidate, pos);
		if (distance2 < nearestDistance2)
		{
			nearest = nearestCandidate;
			nearestDistance2 = distance2;
			nearestSpan = i-1;
			F32 aArcLength = reparam.GetPoint(i-1).arcLength;
			F32 bArcLength = reparam.GetPoint(i).arcLength;
			arcLength = Maths::Lerp(aArcLength, bArcLength, t);
		}
	}

	return arcLength;
}

template<typename CURVE>
F32 FindNearestPointLinear(
	const CURVE& curve,
	const typename CURVE::VecType& pos, 
	typename CURVE::VecType& nearest) 
{	
	F32 nearestDistance2 = FLT_MAX;
	U32 nearestSpan = 0;
	F32 arcLengthResult(0);
	F32 arcLengthCurrent(0);
	for (S32 i = 1; i < curve.GetNumVertices(); ++i)
	{
		typename CURVE::VecType a = curve.GetVertexValue(i-1);
		typename CURVE::VecType b = curve.GetVertexValue(i);
		Maths::HF32 t;
		typename CURVE::VecType nearestCandidate;
		Collision::ClosestPtPointSegment(pos, a, b, nearestCandidate, t);
	
		F32 distance2 = (F32)Maths::DistanceSquared(nearestCandidate, pos);
		if (distance2 < nearestDistance2)
		{
			nearest = nearestCandidate;
			nearestDistance2 = distance2;
			nearestSpan = i-1;
			F32 segmentLength = Helpers::GetLinearSegmentLength(a, b);
			arcLengthResult = arcLengthCurrent + segmentLength * t;
		}

		arcLengthCurrent += Helpers::GetLinearSegmentLength(a, b);
	}

	return arcLengthResult;
}



template<typename CURVE>
S32 FollowerFindNearestReparamIndex(
	const CURVE& curve,
	const C_Reparameterise& reparam,
	const typename CURVE::VecType& position, 
	U32 startIndex)
{
	U32 numKeys = reparam.GetNumPoints();

	F32 closestDist = FLT_MAX;
	U32 closestKey = startIndex;
	for (U32 i=startIndex;i<numKeys;i++)
	{
		typename CURVE::VecType keyframe = ReparamHelpers::GetPointValue(curve, reparam, i);

		typename CURVE::VecType rel = keyframe - position;
		F32 dist = rel.LengthSquared();
		if (dist < closestDist)
		{
			closestDist = dist;
			closestKey = i;
		}
		else
		{
			// keyframe not closer
			return closestKey;
		}
	}

	// reached end of spline
	// are we passed the last node
	typename CURVE::VecType keyframeprev = ReparamHelpers::GetPointValue(curve, reparam, numKeys-2);
	typename CURVE::VecType keyframelast = ReparamHelpers::GetPointValue(curve, reparam, numKeys-1);
	typename CURVE::VecType rel = keyframelast - keyframeprev;
	typename CURVE::VecType relprev = keyframeprev - position;
	if (rel.LengthSquared() < relprev.LengthSquared())
		return -1;
	else
		return numKeys-1;

	return closestKey;
}

template<typename CURVE>
S32 FollowerFindNearestReparamIndexForwardsAndBackwards(
									const CURVE& curve,
									const C_Reparameterise& reparam,
									const typename CURVE::VecType& position, 
									U32 startIndex)
{
	U32 numKeys = reparam.GetNumPoints();

	F32 closestDist = FLT_MAX;
	U32 closestKey = startIndex;
	for (U32 i=startIndex+1;i<numKeys;i++)
	{
		typename CURVE::VecType keyframe = ReparamHelpers::GetPointValue(curve, reparam, i);

		typename CURVE::VecType rel = keyframe - position;
		F32 dist = rel.LengthSquared();
		if (dist < closestDist)
		{
			closestDist = dist;
			closestKey = i;
		}
		else
		{
			// keyframe not closer
			break;
		}
	}

	for (U32 i=startIndex;i>=0;i++)
	{
		typename CURVE::VecType keyframe = ReparamHelpers::GetPointValue(curve, reparam, i);

		typename CURVE::VecType rel = keyframe - position;
		F32 dist = rel.LengthSquared();
		if (dist < closestDist)
		{
			closestDist = dist;
			closestKey = i;
		}
		else
		{
			// keyframe not closer
			break;
		}
	}

	

	return closestKey;
}

template<typename CURVE>
S32 FollowerFindStartKnotIndex(
	const CURVE& curve,
	const typename CURVE::VecType& position)
{
	U32 numKeys = curve.GetNumVertices();

	F32 closestDist = FLT_MAX;
	S32 closestKey = -1;
	for (U32 i = 0; i < numKeys; ++i)
	{
		typename CURVE::VecType keyframe = curve.GetVertexValue(i);
		typename CURVE::VecType rel = keyframe - position;
		F32 dist = rel.LengthSquared();
		if (dist < closestDist)
		{
			closestDist = dist;
			closestKey = i;
		}
	}
	
	return closestKey;
}

template<typename CURVE>
S32 FollowerFindNearestKnotIndex(
								 const CURVE& curve,
								 const typename CURVE::VecType& position, 
								 U32 startIndex)
{
	U32 numKeys = curve.GetNumVertices();

	F32 closestDist = FLT_MAX;
	U32 closestKey = startIndex;
	for (U32 i = startIndex; i < numKeys; ++i)
	{
		typename CURVE::VecType keyframe = curve.GetVertexValue(i);
		typename CURVE::VecType rel = keyframe - position;
		F32 dist = rel.LengthSquared();
		if (dist < closestDist)
		{
			closestDist = dist;
			closestKey = i;
		}
		else
		{
			// keyframe not closer
			return closestKey;
		}
	}
	// reached end of spline
	// are we passed the last node
	typename CURVE::VecType keyframeprev = curve.GetVertexValue(numKeys-2);
	typename CURVE::VecType keyframelast = curve.GetVertexValue(numKeys-1);
	typename CURVE::VecType rel = keyframelast - keyframeprev;
	typename CURVE::VecType relprev = keyframeprev - position;
	if (rel.LengthSquared() < relprev.LengthSquared())
		return -1;
	else
		return numKeys-1;
}



template<typename CURVE>
F32 FollowerFindNearestPoint(
	const CURVE& curve,
	const C_Reparameterise& reparam,
	const typename CURVE::VecType& position, 
	U32 closestKey, 
	typename CURVE::VecType& closestPos,
	typename CURVE::VecType& tangent) 
{	
	F32 arcLength = reparam.GetArcLength();
	S32 numKeys = reparam.GetNumPoints();
	F32 distanceOnSpline=0.0f;	

	if (closestKey == 0) // first key
	{
		typename CURVE::VecType keyframe = ReparamHelpers::GetPointValue(curve, reparam, closestKey);
		F32 keyframeU = reparam.GetPoint(closestKey).u;
		typename CURVE::VecType keyframenext = ReparamHelpers::GetPointValue(curve, reparam, closestKey+1);
		F32 keyframenextU = reparam.GetPoint(closestKey+1).u;
		F32 splinePos;
		closestPos = InterpolatePosLinear(
			keyframe, 
			keyframeU,
			keyframenext,
			keyframenextU,
			position, 
			splinePos);
		tangent = keyframenext - keyframe;
		tangent.Normalise();
		distanceOnSpline = splinePos * arcLength;
	}
	else if (closestKey == numKeys-1) // last key
	{
		typename CURVE::VecType keyframe = ReparamHelpers::GetPointValue(curve, reparam, closestKey);
		F32 keyframeU = reparam.GetPoint(closestKey).u;
		typename CURVE::VecType keyframeprev = ReparamHelpers::GetPointValue(curve, reparam, closestKey-1);
		F32 keyframePrevU = reparam.GetPoint(closestKey-1).u;
		typename CURVE::VecType splineSect = keyframe - keyframeprev;
		typename CURVE::VecType prev = keyframeprev - position;

		tangent = splineSect;
		tangent.Normalise();
		F32 prevLength = prev.LengthSquared();

		if (splineSect.LengthSquared() < prevLength)
		{
			closestPos =  keyframe;
			distanceOnSpline = arcLength;
		}
		else
		{
			F32 splinePos;
			closestPos = InterpolatePosLinear(
				keyframeprev,
				keyframePrevU,
				keyframe,
				keyframeU,
				position, 
				splinePos);
			distanceOnSpline = splinePos * arcLength;
		}
	}
	else
	{
		typename CURVE::VecType keyframe = ReparamHelpers::GetPointValue(curve, reparam, closestKey);
		F32 keyframeU = reparam.GetPoint(closestKey).u;
		typename CURVE::VecType keyframenext = ReparamHelpers::GetPointValue(curve, reparam, closestKey+1);
		F32 keyframenextU = reparam.GetPoint(closestKey+1).u;
		typename CURVE::VecType keyframeprev = ReparamHelpers::GetPointValue(curve, reparam, closestKey-1);
		F32 keyframeprevU = reparam.GetPoint(closestKey-1).u;

		F32 splinePosNext,splinePosPrev;
		const typename CURVE::VecType nextSplinepos = InterpolatePosLinear(
			keyframe,
			keyframeU,
			keyframenext,
			keyframenextU,
			position, 
			splinePosNext);
		const typename CURVE::VecType prevSplinepos = InterpolatePosLinear(
			keyframe,
			keyframeU,
			keyframeprev, 
			keyframeprevU,
			position, 
			splinePosPrev);
		// try it in a linear fashion
		
		const typename CURVE::VecType relNext = nextSplinepos - position;
		const typename CURVE::VecType relPrev = prevSplinepos - position;

		if (relNext.LengthSquared() < relPrev.LengthSquared())
		{
			closestPos = nextSplinepos;
			tangent = keyframenext - keyframe;
			tangent.Normalise();
			distanceOnSpline = splinePosNext * arcLength;
		}
		else
		{
			closestPos = prevSplinepos;
			tangent = keyframe - keyframeprev;
			tangent.Normalise();
			distanceOnSpline = splinePosPrev * arcLength;
		}
	}

	return distanceOnSpline;	
}



template<typename CURVE>
F32 FollowerFindNearestPointLinear(
	const CURVE& curve,
	const typename CURVE::VecType& position, 
	U32 closestKey, 
	typename CURVE::VecType& closestPos,
	typename CURVE::VecType& tangent) 
{	
	F32 arcLength = curve.GetArcLengthLinear();
	S32 nKnots = curve.GetNumVertices();
	F32 distanceOnSpline=0.0f;	

	if (closestKey == 0) // first key
	{
		typename CURVE::VecType keyframe = curve.GetVertexValue(closestKey);
		F32 keyframeU = (F32)closestKey/((F32)nKnots-1);
		typename CURVE::VecType keyframenext = curve.GetVertexValue(closestKey+1);
		F32 keyframenextU = (F32)closestKey+1/((F32)nKnots-1);

		F32 splinePos;
		closestPos = InterpolatePosLinear(
			keyframe, 
			keyframeU,
			keyframenext,
			keyframenextU, 
			position, 
			splinePos);
		tangent = keyframenext - keyframe;
		tangent.Normalise();
		distanceOnSpline = splinePos * arcLength;
	}
	else if (closestKey == nKnots-1) // last key
	{
		typename CURVE::VecType keyframe = curve.GetVertexValue(closestKey);
		F32 keyframeU = (F32)closestKey/((F32)nKnots-1);
		typename CURVE::VecType keyframeprev = curve.GetVertexValue(closestKey-1);
		F32 keyframeprevU = (F32)closestKey-1/((F32)nKnots-1);

		typename CURVE::VecType splineSect = keyframe - keyframeprev;
		typename CURVE::VecType prev = keyframeprev - position;

		tangent = splineSect;
		tangent.Normalise();
		F32 prevLength = prev.Length();
		if (splineSect.Length() < prevLength)
		{
			closestPos =  keyframe;
			distanceOnSpline = arcLength;
		}
		else
		{
			F32 splinePos;
			closestPos = InterpolatePosLinear(
				keyframeprev,
				keyframeprevU, 
				keyframe,
				keyframeU,
				position, 
				splinePos);
			distanceOnSpline = splinePos * arcLength;
		}
	}
	else
	{
		const typename CURVE::VecType keyframe = curve.GetVertexValue(closestKey);
		F32 keyframeU = (F32)closestKey/((F32)nKnots-1);
		const typename CURVE::VecType keyframenext = curve.GetVertexValue(closestKey+1);
		F32 keyframenextU = (F32)closestKey+1/((F32)nKnots-1);
		const typename CURVE::VecType keyframeprev = curve.GetVertexValue(closestKey-1);
		F32 keyframeprevU = (F32)closestKey-1/((F32)nKnots-1);

		F32 splinePosNext,splinePosPrev;
		const typename CURVE::VecType nextSplinepos = InterpolatePosLinear(
			keyframe, 
			keyframeU, 
			keyframenext, 
			keyframenextU,
			position, 
			splinePosNext);
		const typename CURVE::VecType prevSplinepos = InterpolatePosLinear(
			keyframe,
			keyframeU,
			keyframeprev, 
			keyframeprevU,
			position, 
			splinePosPrev);
		// try it in a linear fashion
		
		const typename CURVE::VecType relNext = nextSplinepos - position;
		const typename CURVE::VecType relPrev = prevSplinepos - position;

		if (relNext.LengthSquared() < relPrev.LengthSquared())
		{
			closestPos = nextSplinepos;
			tangent = keyframenext - keyframe;
			tangent.Normalise();
			distanceOnSpline = splinePosNext * arcLength;
		}
		else
		{
			closestPos = prevSplinepos;
			tangent = keyframe - keyframeprev;
			tangent.Normalise();
			distanceOnSpline = splinePosPrev * arcLength;
		}
	}

	return distanceOnSpline;	
}



template<typename CURVE>
F32	FollowerGetArcDistance(
	const CURVE& curve,
	const C_Reparameterise& reparam,
	const typename CURVE::VecType& position, 
	U32 closestKey)
{
	F32 arcLength = reparam.GetArcLength();
	S32 numKeys = reparam.GetNumPoints();
	F32 distanceOnSpline=0.0f;	

	if (closestKey == 0) // first key
	{
		typename CURVE::VecType keyframe = ReparamHelpers::GetPointValue(curve, reparam, closestKey);
		F32 keyframeU = reparam.GetPoint(closestKey).u;
		typename CURVE::VecType keyframenext = ReparamHelpers::GetPointValue(curve, reparam, closestKey+1);
		F32 keyframenextU = reparam.GetPoint(closestKey+1).u;
		F32 splinePos;
		InterpolatePosLinear(
			keyframe, 
			keyframeU,
			keyframenext,
			keyframenextU,
			position, 
			splinePos);
		distanceOnSpline = splinePos * arcLength;
	}
	else if (closestKey == numKeys-1) // last key
	{
		typename CURVE::VecType keyframe = ReparamHelpers::GetPointValue(curve, reparam, closestKey);
		F32 keyframeU = reparam.GetPoint(closestKey).u;
		typename CURVE::VecType keyframeprev = ReparamHelpers::GetPointValue(curve, reparam, closestKey-1);
		F32 keyframePrevU = reparam.GetPoint(closestKey-1).u;
		typename CURVE::VecType splineSect = keyframe - keyframeprev;
		typename CURVE::VecType prev = keyframeprev - position;

		F32 prevLength = prev.Length();
		if (splineSect.Length() < prevLength)
		{
			distanceOnSpline = arcLength;
		}
		else
		{
			F32 splinePos;
			InterpolatePosLinear(
				keyframeprev,
				keyframePrevU,
				keyframe,
				keyframeU,
				position, 
				splinePos);
			distanceOnSpline = splinePos * arcLength;
		}
	}
	else
	{
		typename CURVE::VecType keyframe = ReparamHelpers::GetPointValue(curve, reparam, closestKey);
		F32 keyframeU = reparam.GetPoint(closestKey).u;
		typename CURVE::VecType keyframenext = ReparamHelpers::GetPointValue(curve, reparam, closestKey+1);
		F32 keyframenextU = reparam.GetPoint(closestKey+1).u;
		typename CURVE::VecType keyframeprev = ReparamHelpers::GetPointValue(curve, reparam, closestKey-1);
		F32 keyframeprevU = reparam.GetPoint(closestKey-1).u;

		F32 splinePosNext,splinePosPrev;
		const typename CURVE::VecType nextSplinepos = InterpolatePosLinear(
			keyframe,
			keyframeU,
			keyframenext,
			keyframenextU,
			position, 
			splinePosNext);
		const typename CURVE::VecType prevSplinepos = InterpolatePosLinear(
			keyframe,
			keyframeU,
			keyframeprev, 
			keyframeprevU,
			position, 
			splinePosPrev);
		// try it in a linear fashion
		
		const typename CURVE::VecType relNext = nextSplinepos - position;
		const typename CURVE::VecType relPrev = prevSplinepos - position;

		if (relNext.LengthSquared() < relPrev.LengthSquared())
		{
			distanceOnSpline = splinePosNext * arcLength;
		}
		else
		{
			distanceOnSpline = splinePosPrev * arcLength;
		}
	}

	return distanceOnSpline;
}



template<typename CURVE>
F32	FollowerGetArcDistanceLinear(
	const CURVE& curve,
	const typename CURVE::VecType& position, 
	U32 closestKey)
{
	F32 arcLength = curve.GetArcLengthLinear();
	S32 nKnots = curve.GetNumVertices();
	F32 distanceOnSpline=0.0f;	

	if (closestKey == 0) // first key
	{
		typename CURVE::VecType keyframe = curve.GetVertexValue(closestKey);
		F32 keyframeU = (F32)closestKey/((F32)nKnots-1);
		typename CURVE::VecType keyframenext = curve.GetVertexValue(closestKey+1);
		F32 keyframenextU = (F32)closestKey+1/((F32)nKnots-1);

		F32 splinePos;
		InterpolatePosLinear(
			keyframe, 
			keyframeU,
			keyframenext,
			keyframenextU, 
			position, 
			splinePos);
		distanceOnSpline = splinePos * arcLength;
	}
	else if (closestKey == nKnots-1) // last key
	{
		typename CURVE::VecType keyframe = curve.GetVertexValue(closestKey);
		F32 keyframeU = (F32)closestKey/((F32)nKnots-1);
		typename CURVE::VecType keyframeprev = curve.GetVertexValue(closestKey-1);
		F32 keyframeprevU = (F32)closestKey-1/((F32)nKnots-1);

		typename CURVE::VecType splineSect = keyframe - keyframeprev;
		typename CURVE::VecType prev = keyframeprev - position;

		F32 prevLength = prev.Length();
		if (splineSect.Length() < prevLength)
		{
			distanceOnSpline = arcLength;
		}
		else
		{
			F32 splinePos;
			InterpolatePosLinear(
				keyframeprev,
				keyframeprevU, 
				keyframe,
				keyframeU,
				position, 
				splinePos);
			distanceOnSpline = splinePos * arcLength;
		}
	}
	else
	{
		const typename CURVE::VecType keyframe = curve.GetVertexValue(closestKey);
		F32 keyframeU = (F32)closestKey/((F32)nKnots-1);
		const typename CURVE::VecType keyframenext = curve.GetVertexValue(closestKey+1);
		F32 keyframenextU = (F32)closestKey+1/((F32)nKnots-1);
		const typename CURVE::VecType keyframeprev = curve.GetVertexValue(closestKey-1);
		F32 keyframeprevU = (F32)closestKey-1/((F32)nKnots-1);

		F32 splinePosNext,splinePosPrev;
		const typename CURVE::VecType nextSplinepos = InterpolatePosLinear(
			keyframe, 
			keyframeU, 
			keyframenext, 
			keyframenextU,
			position, 
			splinePosNext);
		const typename CURVE::VecType prevSplinepos = InterpolatePosLinear(
			keyframe,
			keyframeU,
			keyframeprev, 
			keyframeprevU,
			position, 
			splinePosPrev);
		// try it in a linear fashion
		
		const typename CURVE::VecType relNext = nextSplinepos - position;
		const typename CURVE::VecType relPrev = prevSplinepos - position;

		if (relNext.LengthSquared() < relPrev.LengthSquared())
		{
			distanceOnSpline = splinePosNext * arcLength;
		}
		else
		{
			distanceOnSpline = splinePosPrev * arcLength;
		}
	}

	return distanceOnSpline;		
}



template<typename CURVE>
void GetForwardFacingSplinePos(
	Maths::C_QuatTrans& quatTrans,
	const CURVE& curve,
	F32 u,
	F32 uNext,
	bool clampEnd)
{
	const typename CURVE::VecType pos = curve.Evaluate(u);
	const typename CURVE::VecType nextpos = curve.Evaluate(uNext);

	if (uNext < 1.0f || !clampEnd)
	{
		typename CURVE::VecType zaxis = nextpos - pos;
		zaxis.Normalise();

		const typename CURVE::VecType upVector(0.0f, 1.0f, 0.0f);
		typename CURVE::VecType right = (upVector ^ zaxis);
		typename CURVE::VecType up = (zaxis ^ right);

		right.Normalise();
		up.Normalise();
		Maths::HMat33 mat;
		mat.AZ(zaxis);
		mat.AX(right);
		mat.AY(up);
		quatTrans.Quaternion(Maths::C_Quaternion(mat));
		quatTrans.Translation(pos);
	}
	else 
	{
		// at the end do not bother changing orientation as near to the end
		quatTrans.Translation(pos);
	}
}
