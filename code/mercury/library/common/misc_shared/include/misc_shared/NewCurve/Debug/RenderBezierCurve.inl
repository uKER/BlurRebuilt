template<typename STORAGE>
static void RenderBezierCurve(
	DbUtil::I_DebugRenderPtr debugRenderPtr,
	const C_BezierCurve<Maths::HVec3, STORAGE>& curve,
	F32 paramLength, 
	U32 nSteps,
	const Maths::HMat43& transform,
	const Maths::RGBA& segA,
	const Maths::RGBA& segB,
	const Maths::RGBA& tangent)
{
	const C_CurveData<C_BezierCurveKnot<Maths::HVec3> >* pData = curve.GetData();
	if (!pData || pData->GetNumKnots() < 2) return;

	RenderUStep(debugRenderPtr, curve, paramLength, nSteps, transform, segA, segB);

	Maths::HVec3 prevPosition = curve.Evaluate(curve.GetMinU());

	F32 tStep = paramLength/nSteps;
	F32 t = tStep;

	U32 nSegments = pData->GetNumKnots()-1;
	for(U32 i = 0; i < nSegments; ++i)
	{
		const typename STORAGE::KnotType& knot0 = pData->AcquireKnot(i);
		const typename STORAGE::KnotType& knot1 = pData->AcquireKnot(i+1);

		debugRenderPtr->Draw3dLine(
			knot0.value*transform, knot0.ctrlOut*transform, tangent);

		debugRenderPtr->Draw3dLine(
			knot1.value*transform, knot1.ctrlIn*transform, tangent);

		pData->ReleaseKnot();
		pData->ReleaseKnot();
	}
}


