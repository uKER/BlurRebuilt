template<typename CURVE>
static void RenderHermiteSpline3D(
	DbUtil::I_DebugRenderPtr debugRenderPtr,
	const CURVE& curve,
	F32 paramLength, 
	U32 nSteps,
	const Maths::HMat43& transform, 
	const Maths::RGBA& segA,
	const Maths::RGBA& segB,
	const Maths::RGBA& tangent)
{
	const typename CURVE::StorageType* pData = curve.GetData();
	U32 nKnots = pData->GetNumKnots();
	if (!pData || nKnots < 2) return;

	RenderUStep(debugRenderPtr, curve, paramLength, nSteps, transform, segA, segB);

	Maths::HVec3 prevPosition = curve.Evaluate(curve.GetMinU());

	F32 tStep = paramLength/nSteps;
	F32 t = tStep;

	U32 nSegments = pData->GetNumKnots()-1;
	for(U32 i = 0; i < nSegments; ++i)
	{
		const typename CURVE::StorageType::KnotType& knot0 = pData->AcquireKnot(i);
		const typename CURVE::StorageType::KnotType& knot1 = pData->AcquireKnot(i+1);
		F32 knot0u = knot1.GetU(i, nKnots);
		F32 knot1u = knot1.GetU(i+1, nKnots);

		Maths::HVec3 control0;
		Maths::HVec3 control1;
		curve.GenerateCtrlPts(control0, control1, knot0, knot0u, knot1, knot1u);

		debugRenderPtr->Draw3dLine(
			knot0.value*transform, control0*transform, tangent);

		debugRenderPtr->Draw3dLine(
			knot1.value*transform, control1*transform, tangent);

		pData->ReleaseKnot();
		pData->ReleaseKnot();
	}
}

template<typename CURVE>
static void RenderHermiteSpline1D(
	DbUtil::I_DebugRenderPtr debugRenderPtr,
	const CURVE& curve, 
	F32 paramLength, 
	U32 nSteps,
	const Maths::HMat43& transform,
	const Maths::RGBA& segA, 
	const Maths::RGBA& segB,
	const Maths::RGBA& tangent)
{
	const typename CURVE::StorageType* pData = curve.GetData();
	if (!pData || pData->GetNumKnots() < 2) return;

	RenderUStep(debugRenderPtr, curve, paramLength, nSteps, transform, segA, segB);

	U32 nKnots = pData->GetNumKnots();
	U32 nSegments = nKnots-1;
	for(U32 i = 0; i < nSegments; ++i)
	{
		const typename CURVE::StorageType::KnotType& knot0 = pData->AcquireKnot(i);
		const typename CURVE::StorageType::KnotType& knot1 = pData->AcquireKnot(i+1);
		F32 knot0U = knot1.GetU(i, nKnots);
		F32 knot1U = knot1.GetU(i+1, nKnots);

		Maths::HVec3 knot0Pos(knot0U, 0, knot0.value);
		Maths::HVec3 knot1Pos(knot1U, 0, knot1.value);
		F32 segmentPeriod = knot1U - knot0U;

		Maths::HVec3 control0 = Helpers::OutTangentToControlPoint1dTo3d(knot0Pos, knot0.tangentOut, knot0.tangentScale, segmentPeriod);
		debugRenderPtr->Draw3dLine(
			knot0Pos*transform, control0*transform, tangent);

		Maths::HVec3 control1 = Helpers::InTangentToControlPoint1dTo3d(knot1Pos, knot1.tangentIn, knot1.tangentScale, segmentPeriod);
		debugRenderPtr->Draw3dLine(
			knot1Pos*transform, control1*transform, tangent);

		pData->ReleaseKnot();
		pData->ReleaseKnot();
	}
}

