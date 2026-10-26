#include "SharedActorsPch.h"

#include <Xt/XtGame.h>
#include <Core/UtMaths.h>
#include <Core/XtStdx.h>
#include <Btl/Containers/Vector.h>

#include <System/Debug/I_DebugRender.h>

#include <Shared_Actors/TypeWrappers/SplineVec3Wrapper.h>
#include <Gravity/NarrowPhase/PrimitiveTest.h>
#include <Misc_shared/NewCurveHermite.h>
#include <Misc_Shared/NewCurve/Actor/ActorSplineTypes.h>
#include <Misc_Shared/NewCurve/Actor/ActorHelpers.h>
#include <Misc_Shared/NewCurve/ArcIterator.h>
#include <Misc_Shared/NewCurve/Debug/DebugRender.h>
#include <Misc_Shared/NewCurve/Debug/RenderHermiteSpline.h>

#ifdef SHARED_ACTORS_OPTIMIZE_OFF
#pragma optimize( "", off ) 
#endif

Debug::Bool g_DebugRenderSplineActor( "ActorDebug.Actors.SplineVec3.DebugRender.Enable", false );
Debug::Bool g_DebugRenderDisplayReparam( "ActorDebug.Actors.SplineVec3.DebugRender.DisplayReparam", true );
Debug::Float g_DebugRenderSplineResolution( "ActorDebug.Actors.SplineVec3.DebugRender.DebugResolution", 4.f, Debug::Range(1.f, 20.f));
Debug::Float g_SplineLookAheadFactorUseCalcUHack( "ActorDebug.Actors.SplineVec3.SplineLookAheadFactorUseCalcUHack", 2.0f, Debug::Range(0.0f, 10.f));
Debug::Float g_SplineLookAheadFactorArcLength( "ActorDebug.Actors.SplineVec3.SplineLookAheadFactorArcLength", 1.f, Debug::Range(0.0f, 10.f));

namespace SharedActors
{

Maths::HVec3 TestLinearSplineFunction(
	const Curve::C_Hermite3DActor::KnotType* keyA, 
	const Curve::C_Hermite3DActor::KnotType* keyB, 
	const Maths::HVec3 pos, 
	F32& splinePos)
{
	Maths::HVec3 keyPosA(keyA->value);
	Maths::HVec3 keyPosB(keyB->value);
	Maths::HVec3 keyVect = keyPosB - keyPosA;
	F32 sectLength = keyVect.Length();
	Maths::HVec3 AtoPos = pos - keyPosA;
	F32 posLength = AtoPos.Length();
	AtoPos.Normalise();
	keyVect.Normalise();
	F32 dot =  AtoPos| keyVect;
	// if pos is behind then closest pos is keyA
	dot = Maths::Max(dot, 0.0f);
	F32 scaleLength = posLength * dot;
	F32 AtoBposition = keyB->u - keyA->u;
	splinePos = keyA->u + ((scaleLength/sectLength) * AtoBposition);
	return (keyPosA + (scaleLength * keyVect));
}

C_SplineVec3Wrapper::C_SplineVec3Wrapper()
{
	m_pCurveData = new Curve::C_Hermite3DActorDataProxy;
	m_pCurveEval = new Curve::C_ActorCurve3D();
	m_isClosed = false;
}
									
C_SplineVec3Wrapper::~C_SplineVec3Wrapper()
{
	SAFE_DELETE_PTR(m_pCurveData);
	SAFE_DELETE_PTR(m_pCurveEval);
}

bool C_SplineVec3Wrapper::Initialise(const XtSplineVec3* pXtData, const Maths::HMat43& createMat)
{
	m_pCurveData->SetData(pXtData);

	BIZ_ASSERT(pXtData->knots.count > 1);

	Curve::C_HermiteSplineFlags flags;
	//Set to none, as we secretly handle u values within the xt proxy.
	flags.calcKnotU = Curve::E_ProcessU_None;
	flags.calcTangents = false;
	Curve::C_Hermite3DActor* pSplineEvaluator = new Curve::C_Hermite3DActor(flags);
	pSplineEvaluator->SetData(m_pCurveData);
	pSplineEvaluator->Process();
	//Transfer ownership.
	m_pCurveEval->SetCurve(pSplineEvaluator);

	F32 resolution;

	switch(pXtData->reparamRes)
	{
	case XtSplineReparamHigh:
		resolution = Curve::kReparamResHigh;
		break;
	//Medium is default for new splines, and splines predating the ability to set the resolution.
	case XtSplineReparamMedium:
		resolution = Curve::kReparamResMedium;
		break;
	case XtSplineReparamLow:
		resolution = Curve::kReparamResLow;
		break;
	case XtSplineReparamDefault:
		resolution = Curve::kReparamResDefault;
		break;
	case XtSplineReparamNone:
	default:
		resolution = 0.f;
		break;
	}

	// Scale the resolution by the minimum segment length on the original spline.
	resolution *= 1.f / (pSplineEvaluator->GetMinSegmentLength() + 1.f);

	U32 nReparamSteps = (F32)(pSplineEvaluator->GetArcLengthLinear() * resolution);

	m_pCurveEval->Process(nReparamSteps);
	m_isClosed = pXtData->closed;

	XtArray<XtKnotVec3>& knots = const_cast<XtArray<XtKnotVec3>&>(pXtData->knots);
	S32 nKnots = knots.count;

	//Slightly evil transformation of xt data into world space.
	for (S32 i = 0; i < nKnots; ++i)
	{
		XtKnotVec3& xtKnot = const_cast<XtKnotVec3&>(knots[i]);
		Maths::HVec3 value = Load(xtKnot.position) * createMat;
		Maths::HVec3 tangentIn = Load(xtKnot.tangentIn) * createMat.Rotation();
		Maths::HVec3 tangentOut = Load(xtKnot.tangentOut) * createMat.Rotation();

		xtKnot.position = Store(value);
		xtKnot.tangentIn = Store(tangentIn);
		xtKnot.tangentOut = Store(tangentOut);
	}

	return true;
}

void C_SplineVec3Wrapper::DebugRender(DbUtil::I_DebugRender* pDebugRenderPtr) const
{
	if (!g_DebugRenderSplineActor)
		return;

	Maths::HMat43 id;
	id.SetIdentity();
	if (g_DebugRenderDisplayReparam)
	{
		RenderReparamData(
			pDebugRenderPtr,
			*m_pCurveEval->GetCurve(),
			m_pCurveEval->GetReparam(),
			1.f, 
			id,
			Maths::RGBA(255, 255, 255),
			Maths::RGBA(0, 0, 200));
	}
	else
	{
		Curve::RenderHermiteSpline3D(
			pDebugRenderPtr, 
			*(Curve::C_Hermite3DActor*)m_pCurveEval->GetCurve(), 
			1.f,
			m_pCurveData->GetNumKnots()*(U32)g_DebugRenderSplineResolution, 
			id,
			Maths::RGBA(255, 255, 255),
			Maths::RGBA(0, 0, 200),
			Maths::RGBA(255, 255, 128));
	}

	Curve::RenderKnots(
		pDebugRenderPtr, 
		*m_pCurveData,
		id,
		Maths::RGBA(255, 255, 128));
}

void C_SplineVec3Wrapper::GetForwardFacingSplinePos_UseCalcUHack(
	Maths::C_QuatTrans& quatTrans,
	F32 progress, 
	F32 speed,
	F32 deltaTime) const
{
	F32 arcLength = m_pCurveEval->GetArcLengthLinear();
	F32 prog = speed * deltaTime / arcLength;
	const Maths::HVec3 pos = m_pCurveEval->Evaluate(progress);
	F32 nextProg = progress+(prog*g_SplineLookAheadFactorUseCalcUHack);
	const Maths::HVec3 nextpos = m_pCurveEval->Evaluate(nextProg);

	if (nextProg < 1.0f)
	{
		Maths::HVec3 zaxis = nextpos - pos;
		zaxis.Normalise();

		const Maths::HVec3 upVector(0.0f, 1.0f, 0.0f);
		Maths::HVec3 right = (upVector ^ zaxis);
		Maths::HVec3 up = (zaxis ^ right);

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

F32 C_SplineVec3Wrapper::FollowerFindNearestPoint_UseCalcUHack(
	const Maths::C_Vertex& position, 
	U32 closestKey, 
	Maths::HVec3 &closestPos,
	Maths::HVec3 &tangent) const 
{	
	F32 arcLength = m_pCurveEval->GetArcLengthLinear();
	S32 numKeys = m_pCurveData->GetNumKnots();
	F32 distanceOnSpline=0.0f;	

	if (closestKey == 0) // first key
	{
		const Curve::C_Hermite3DActor::KnotType& keyframe = m_pCurveData->AcquireKnot(closestKey);
		const Curve::C_Hermite3DActor::KnotType& keyframenext = m_pCurveData->AcquireKnot(closestKey+1);
		F32 splinePos;
		closestPos = TestLinearSplineFunction(&keyframe, &keyframenext, position, splinePos);
		tangent = keyframenext.value - keyframe.value;
		tangent.Normalise();
		distanceOnSpline = splinePos * arcLength;

		m_pCurveData->ReleaseKnot();
		m_pCurveData->ReleaseKnot();
	}
	else if (closestKey == numKeys-1) // last key
	{
		const Curve::C_Hermite3DActor::KnotType& keyframe = m_pCurveData->AcquireKnot(closestKey);
		const Curve::C_Hermite3DActor::KnotType& keyframeprev = m_pCurveData->AcquireKnot(closestKey-1);
		Maths::HVec3 splineSect = keyframe.value - keyframeprev.value;
		Maths::HVec3 prev = keyframeprev.value - position;

		tangent = splineSect;
		tangent.Normalise();
		F32 prevLength = prev.Length();
		if (splineSect.Length() < prevLength)
		{
			closestPos =  keyframe.value;
			distanceOnSpline = arcLength;
		}
		else
		{
			F32 splinePos;
			closestPos = TestLinearSplineFunction(&keyframeprev, &keyframe, position, splinePos);
			distanceOnSpline = splinePos * arcLength;
		}

		m_pCurveData->ReleaseKnot();
		m_pCurveData->ReleaseKnot();
	}
	else
	{
		const Curve::C_Hermite3DActor::KnotType& keyframe = m_pCurveData->AcquireKnot(closestKey);
		const Curve::C_Hermite3DActor::KnotType& keyframenext = m_pCurveData->AcquireKnot(closestKey+1);
		const Curve::C_Hermite3DActor::KnotType& keyframeprev = m_pCurveData->AcquireKnot(closestKey-1);

		F32 splinePosNext,splinePosPrev;
		const Maths::HVec3 nextSplinepos = TestLinearSplineFunction(&keyframe, &keyframenext, position, splinePosNext);
		const Maths::HVec3 prevSplinepos = TestLinearSplineFunction(&keyframe, &keyframeprev, position, splinePosPrev);
		// try it in a linear fashion
		
		const Maths::HVec3 relNext = nextSplinepos - position;
		const Maths::HVec3 relPrev = prevSplinepos - position;

		if (relNext.LengthSquared() < relPrev.LengthSquared())
		{
			closestPos = nextSplinepos;
			tangent = keyframenext.value - keyframe.value;
			tangent.Normalise();
			distanceOnSpline = splinePosNext * arcLength;
		}
		else
		{
			closestPos = prevSplinepos;
			tangent = keyframe.value - keyframeprev.value;
			tangent.Normalise();
			distanceOnSpline = splinePosPrev * arcLength;
		}

		m_pCurveData->ReleaseKnot();
		m_pCurveData->ReleaseKnot();
		m_pCurveData->ReleaseKnot();
	}

	return distanceOnSpline;	
}

F32 C_SplineVec3Wrapper::FollowerGetArcDistance_UseCalcUHack(const Maths::C_Vertex& position, U32 closestKey) const 
{	
	S32 numKeys = m_pCurveData->GetNumKnots();
	F32 distanceOnSpline=0.0f;
	F32 arcLength = m_pCurveEval->GetArcLengthLinear();

	if (closestKey == 0) // first key
	{
		const Curve::C_Hermite3DActor::KnotType& keyframe = m_pCurveData->AcquireKnot(closestKey);
		const Curve::C_Hermite3DActor::KnotType& keyframenext = m_pCurveData->AcquireKnot(closestKey+1);
		F32 splinePos;
		TestLinearSplineFunction(&keyframe, &keyframenext, position, splinePos);
		distanceOnSpline = splinePos * arcLength;
		m_pCurveData->ReleaseKnot();
		m_pCurveData->ReleaseKnot();
	}
	else if (closestKey == numKeys-1) // last key
	{
		const Curve::C_Hermite3DActor::KnotType& keyframe = m_pCurveData->AcquireKnot(closestKey);
		const Curve::C_Hermite3DActor::KnotType& keyframeprev = m_pCurveData->AcquireKnot(closestKey-1);	
		Maths::HVec3 splineSect = keyframe.value - keyframeprev.value;
		Maths::HVec3 prev = keyframeprev.value - position;
		
		F32 prevLength = prev.Length();
		if (splineSect.Length() < prevLength)
		{
			distanceOnSpline = arcLength;
		}
		else
		{
			F32 splinePos;
			TestLinearSplineFunction(&keyframeprev, &keyframe, position, splinePos);
			distanceOnSpline = splinePos * arcLength;
		}
		m_pCurveData->ReleaseKnot();
		m_pCurveData->ReleaseKnot();
	}
	else
	{
		const Curve::C_Hermite3DActor::KnotType& keyframe = m_pCurveData->AcquireKnot(closestKey);
		const Curve::C_Hermite3DActor::KnotType& keyframenext = m_pCurveData->AcquireKnot(closestKey+1);
		const Curve::C_Hermite3DActor::KnotType& keyframeprev = m_pCurveData->AcquireKnot(closestKey-1);

		F32 splinePosNext,splinePosPrev;
		const Maths::HVec3 nextSplinepos = TestLinearSplineFunction(&keyframe, &keyframenext, position, splinePosNext);
		const Maths::HVec3 prevSplinepos = TestLinearSplineFunction(&keyframe, &keyframeprev, position, splinePosPrev);
		// try it in a linear fashion

		const Maths::HVec3 relNext = nextSplinepos - position;
		const Maths::HVec3 relPrev = prevSplinepos - position;

		if (relNext.LengthSquared() < relPrev.LengthSquared())
			distanceOnSpline = splinePosNext * arcLength;
		else
			distanceOnSpline = splinePosPrev * arcLength;

		m_pCurveData->ReleaseKnot();
		m_pCurveData->ReleaseKnot();
		m_pCurveData->ReleaseKnot();
	}

	return distanceOnSpline;	
}

F32 C_SplineVec3Wrapper::GetForwardFacingSplinePos(
	Maths::C_QuatTrans& quatTrans,
	F32 progress, 
	F32 offset,
	F32 speed,
	F32 deltaTime,
	bool wrap) const
{
	F32 arcLength = m_pCurveEval->GetArcLength();

	F32 arcProgress = offset + arcLength * progress;
	if (arcProgress > arcLength)
		arcProgress = arcProgress - arcLength;
	const Maths::HVec3 pos = m_pCurveEval->EvaluateArcLength(arcProgress);
	
	F32 deltaDistance = speed * deltaTime;
	F32 nextProg = arcProgress+(deltaDistance*g_SplineLookAheadFactorArcLength);
	if (wrap && nextProg > arcLength)
		nextProg = nextProg - arcLength;
	const Maths::HVec3 nextpos = m_pCurveEval->EvaluateArcLength(nextProg);

	if (nextProg < arcLength)
	{
		Maths::HVec3 zaxis = nextpos - pos;
		zaxis.Normalise();

		const Maths::HVec3 upVector(0.0f, 1.0f, 0.0f);
		Maths::HVec3 right = (upVector ^ zaxis);
		Maths::HVec3 up = (zaxis ^ right);

		right.Normalise();
		up.Normalise();
		Maths::HMat33 mat;
		mat.AZ(zaxis);
		mat.AX(right);
		mat.AY(up);
		quatTrans.Quaternion(Maths::C_Quaternion(mat));
		quatTrans.Translation(pos);
		return arcProgress;
	}
	else
	{
		// at the end do not bother changing orientation as near to the end
		quatTrans.Translation(pos);
		return arcLength;
	}
}

F32 C_SplineVec3Wrapper::GetForwardFacingSplinePos(Maths::C_QuatTrans& quatTrans, F32 arcProgress) const
{
	const F32		kNextFudgeFactor = 0.2f;			// w 0.02f;	Was causing judder
	F32				arcLength = m_pCurveEval->GetArcLength();
	Maths::HVec3	pos,start,end;

	if (arcProgress > arcLength)
		arcProgress = arcLength;

	pos = m_pCurveEval->EvaluateArcLength(arcProgress);

	F32 nextProg = arcProgress + kNextFudgeFactor;
	if (nextProg >= arcLength)
	{
		start = m_pCurveEval->EvaluateArcLength(arcLength - kNextFudgeFactor);
		end = m_pCurveEval->EvaluateArcLength(arcLength);
	}
	else
	{
		start = pos;
		end = m_pCurveEval->EvaluateArcLength(nextProg);
	}

	Maths::HVec3 zaxis = end - start;
	zaxis.Normalise();

	const Maths::HVec3 upVector(0.0f, 1.0f, 0.0f);
	Maths::HVec3 right = (upVector ^ zaxis);
	Maths::HVec3 up = (zaxis ^ right);

	right.Normalise();
	up.Normalise();
	Maths::HMat33 mat;
	mat.AZ(zaxis);
	mat.AX(right);
	mat.AY(up);
	quatTrans.Quaternion(Maths::C_Quaternion(mat));
	quatTrans.Translation(pos);

	return(arcProgress);
}

/*
F32 C_SplineVec3Wrapper::GetForwardFacingSplinePos(
	Maths::C_QuatTrans& quatTrans,
	F32 arcProgress) const
{
	F32 arcLength = m_pCurveEval->GetArcLength();

	const Maths::HVec3 pos = m_pCurveEval->EvaluateArcLength(arcProgress);
	
	const F32 kNextFudgeFactor = 0.02f;
	F32 nextProg = arcProgress + kNextFudgeFactor;
	if (nextProg < arcLength)
	{
		const Maths::HVec3 nextpos = m_pCurveEval->EvaluateArcLength(nextProg);

		Maths::HVec3 zaxis = nextpos - pos;
		zaxis.Normalise();

		const Maths::HVec3 upVector(0.0f, 1.0f, 0.0f);
		Maths::HVec3 right = (upVector ^ zaxis);
		Maths::HVec3 up = (zaxis ^ right);

		right.Normalise();
		up.Normalise();
		Maths::HMat33 mat;
		mat.AZ(zaxis);
		mat.AX(right);
		mat.AY(up);
		quatTrans.Quaternion(Maths::C_Quaternion(mat));
		quatTrans.Translation(pos);
		return arcProgress;
	}
	else
	{
		// at the end do not bother changing orientation as near to the end
		quatTrans.Translation(pos);
		return arcLength;
	}
}
*/

F32 C_SplineVec3Wrapper::GetSplinePos(
	Maths::HVec3& trans,
	F32 progress, 
	F32 offset,
	F32 speed,
	F32 deltaTime) const
{
	F32 arcLength = m_pCurveEval->GetArcLength();

	F32 arcProgress = offset + arcLength * progress;
	if (arcProgress > arcLength)
		arcProgress = arcProgress - arcLength;
	trans = m_pCurveEval->EvaluateArcLength(arcProgress);
	return arcProgress;
}

S32 C_SplineVec3Wrapper::FollowerFindNearestKnotIndex(const Maths::C_Vertex& position, U32 startIndex) const
{
	return Curve::Actor::FollowerFindNearestKnotIndex(*m_pCurveEval->GetCurve(), position, startIndex);
}

S32 C_SplineVec3Wrapper::FollowerFindStartKnotIndex(const Maths::C_Vertex& position) const
{
	return Curve::Actor::FollowerFindStartKnotIndex(*m_pCurveEval->GetCurve(), position);
}


}

#ifdef SHARED_ACTORS_OPTIMIZE_OFF
#pragma optimize ("", on)
#endif
