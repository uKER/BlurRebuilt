template<typename STORAGE>
void RenderKnots(
	DbUtil::I_DebugRenderPtr debugRenderPtr,
	const STORAGE& curveData, 
	const Maths::HMat43& transform,
	const Maths::RGBA& colour)
{
	S32 nKnots = curveData.GetNumKnots();
	if (nKnots == 0)
		return;

	S32 i = 0;
	for(; i < nKnots-1; ++i)
	{
		const typename STORAGE::KnotType& knot0 = curveData.AcquireKnot(i);
		const typename STORAGE::KnotType& knot1 = curveData.AcquireKnot(i+1);
		debugRenderPtr->Draw3dCross(
			knot0.value*transform, 0.1f, colour);
		curveData.ReleaseKnot();
		curveData.ReleaseKnot();
	}
	const typename STORAGE::KnotType& knot = curveData.AcquireKnot(i);
	debugRenderPtr->Draw3dCross(knot.value*transform, 0.1f, colour);
	curveData.ReleaseKnot();
}

template<typename STORAGE>
void RenderLinear(
	DbUtil::I_DebugRenderPtr debugRenderPtr,
	const STORAGE& curveData, 
	const Maths::HMat43& transform,
	const Maths::RGBA& segA, 
	const Maths::RGBA& segB)
{
	S32 nKnots = curveData.GetNumKnots();
	for(S32 i = 0; i < nKnots-1; ++i)
	{
		const typename STORAGE::KnotType& knot0 = curveData.AcquireKnot(i);
		const typename STORAGE::KnotType& knot1 = curveData.AcquireKnot(i+1);
		Maths::RGBA colour = i&1 ? segA : segB;
		debugRenderPtr->Draw3dLine(
			knot0.value*transform, knot1.value, colour);
		curveData.ReleaseKnot();
		curveData.ReleaseKnot();
	}
}

void RenderUStep(
	DbUtil::I_DebugRenderPtr debugRenderPtr,
	const I_Curve<Maths::HVec3>& curve,
	F32 paramLength, 
	U32 nSteps,
	const Maths::HMat43& transform,
	const Maths::RGBA& segA, 
	const Maths::RGBA& segB)
{
	if (curve.GetNumVertices() == 0)
		return;

	F32 tMin = curve.GetMinU();
	F32 tMax = curve.GetMaxU();
	F32 tSpan = tMax - tMin;

	Maths::HVec3 prevPosition = curve.Evaluate(tMin);

	F32 tStep = (tSpan*paramLength)/nSteps;
	F32 t = tMin+tStep;
	for(U32 i = 1; i <= nSteps; ++i, t+=tStep)
	{
		Maths::HVec3 nextPosition = curve.Evaluate(t);

		Maths::RGBA colour = i&1 ? segA : segB;
		debugRenderPtr->Draw3dLine(prevPosition*transform, nextPosition*transform, colour);
		prevPosition = nextPosition;

		if (t >= tSpan+tMin)
			break;
	}
}

void RenderUStep(
	DbUtil::I_DebugRenderPtr debugRenderPtr,
	const I_Curve<F32>& curve,
	F32 paramLength, 
	U32 nSteps,
	const Maths::HMat43& transform,
	const Maths::RGBA& segA,
	const Maths::RGBA& segB)
{
	if (curve.GetNumVertices() == 0)
		return;

	F32 tMin = curve.GetMinU();
	F32 tMax = curve.GetMaxU();
	F32 tSpan = tMax - tMin;

	Maths::HVec3 prevPosition(tMin, 0, curve.Evaluate(tMin));

	F32 tStep = (tSpan*paramLength)/nSteps;
	F32 t = tMin+tStep;
	for(U32 i = 1; i <= nSteps; ++i, t+=tStep)
	{
		Maths::HVec3 nextPosition(t, 0, curve.Evaluate(t));

		Maths::RGBA colour = i&1 ? segA : segB;
		debugRenderPtr->Draw3dLine(prevPosition*transform, nextPosition*transform, colour);
		prevPosition = nextPosition;

		if (t >= tSpan+tMin)
			break;
	}
}

void RenderArcStep(
	DbUtil::I_DebugRenderPtr debugRenderPtr,
	const I_Curve<Maths::HVec3>& curve, 
	const C_Reparameterise& reparam,
	F32 paramLength,
	F32 stepSize,
	const Maths::HMat43& transform,
	const Maths::RGBA& segA, 
	const Maths::RGBA& segB)
{
	if (stepSize == 0 || reparam.GetNumPoints() == 0)
		return;

	F32 endLength = reparam.GetArcLength() * paramLength;
	U32 endStep = (U32)(endLength / stepSize);

	//Let's iterate using an arc iterator.
	{
		Curve::C_ArcIterator<Maths::HVec3> it(curve, reparam, stepSize);
		Maths::HVec3 prevPosition = *it;
		++it;

		//If we set i to 0 then we'll end up with a 'filler' line at the end to compensate for the fact that
		//our step size * the number of steps may not equal the arc length.
		for (U32 i = 1; it; ++it, ++i)
		{
			Maths::RGBA colour = i&1 ? segA : segB;
			debugRenderPtr->Draw3dLine(prevPosition*transform, (*it)*transform, colour);
			prevPosition = *it;	
			if (i >= endStep)
				break;
		}
	}
}


void RenderReparamData(
	DbUtil::I_DebugRenderPtr debugRenderPtr,
	const I_Curve<Maths::HVec3>& curve, 
	const C_Reparameterise& reparam,
	F32 paramLength,
	const Maths::HMat43& transform,
	const Maths::RGBA& segA, 
	const Maths::RGBA& segB)
{
	if (reparam.GetNumPoints() == 0)
		return;

	F32 endLength = reparam.GetArcLength() * paramLength;
	Maths::HVec3 prevPosition = ReparamHelpers::GetPointValue(curve, reparam, 0);
	for (U32 i = 0; i < reparam.GetNumPoints(); ++i)
	{
		Maths::HVec3 nextPosition = ReparamHelpers::GetPointValue(curve, reparam, i);
		F32 arcLength = reparam.GetPoint(i).arcLength;
		if (arcLength > endLength)
			break;

		Maths::RGBA colour = i&1 ? segA : segB;
		debugRenderPtr->Draw3dLine(prevPosition*transform, nextPosition*transform, colour);
		prevPosition = nextPosition;
	}
}

