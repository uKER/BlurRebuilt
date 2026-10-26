#include "stdafx.h"

#include <Core/Maths.h>
#include <Core/maths/hvec/New/IncludeGeometry.h>
#include <Core/maths/hvec/New/IncludeGeometry.inl>
#include <Core/UtMaths.h>
#include <Btl/Containers/vector.h>
#include <System/Debug/I_DebugRender.h>
#include <System/Debug/Debug.h>

#include <Misc_Shared/NewCurve/Definitions.h>
#include <Misc_Shared/NewCurve/CurveHelpers.h>
#include <Misc_Shared/NewCurve/Reparameterise.h>
#include <Misc_Shared/NewCurve/ReparamHelpers.h>
#include <Misc_Shared/NewCurve/CurveData.h>
#include <Misc_Shared/NewCurve/Curve.h>
#include <Misc_Shared/NewCurve/BezierCurve.h>
#include <Misc_Shared/NewCurve/CardinalSpline.h>
#include <Misc_Shared/NewCurve/CatmullRomSpline.h>
#include <Misc_Shared/NewCurve/HermitePolicies.h>
#include <Misc_Shared/NewCurve/HermiteSpline.h>
#include <Misc_Shared/NewCurve/Polyline.h>
#include <Misc_Shared/NewCurve/NurbSplineData.h>
#include <Misc_Shared/NewCurve/NurbSpline.h>
#include <Misc_Shared/NewCurve/MayaHermiteSpline.h>
#include <Misc_Shared/NewCurve/ArcIterator.h>
#include <Misc_Shared/NewCurve/CurveWrapper.h>
#include <Misc_Shared/NewCurve/BSpline.h>

#include <Misc_Shared/NewCurve/Debug/DebugRender.h>
#include <Misc_Shared/NewCurve/Debug/RenderBezierCurve.h>
#include <Misc_Shared/NewCurve/Debug/RenderHermiteSpline.h>
#include <Misc_Shared/NewCurve/Debug/HelloWorld.h>

#include <Gravity/NarrowPhase/Primitive.h>
#include <Gravity/NarrowPhase/CollisionResult.h>
#include <Gravity/NarrowPhase/PrimitiveTest.h>
#include <Gravity/NarrowPhase/Primitive.inl>
#include <Misc_Shared/NewCurve/Actor/ActorHelpers.h>

Debug::Int g_NewCurveReparamRes("MiscShared.NewCurve.ReparamRes", 5, Debug::Range(1, 20));
Debug::Int g_NewCurveUSteps("MiscShared.NewCurve.USize", 40, Debug::Range(1, 100));
Debug::Float g_NewCurveArcStepSize("MiscShared.NewCurve.ArcStepSize", 0.2f, Debug::Range(0.01f, 1.f));
Debug::Float g_NewCurveTestPoint("MiscShared.NewCurve.TestPoint", 0.f, Debug::Range(-1.0f, 2.f));

namespace Curve
{

void HelloHermiteSpline(F32 paramLength)
{
	C_CurveData<C_HermiteSplineKnot<Maths::HVec3> > data;
	Btl::Vector<C_HermiteSplineKnot<Maths::HVec3> >& samples = data.GetKnots();

	U32 nSamples = 3;
	samples.Resize(nSamples);
	samples[0].value = Maths::HVec3(0, 0, 0.34f);
//	samples[1].value = Maths::HVec3(0.5f, 0, 0.34f);
	samples[1].value = Maths::HVec3(0.5f, 0, 0.6f);
//	samples[2].value = Maths::HVec3(1.f, 0, 0.2f);
	samples[2].value = Maths::HVec3(1.f, 0, 0.34f);

	samples[0].tangentIn = Maths::HVec3(0,0,0);
	samples[0].tangentOut = Helpers::ControlPointToOutTangent(samples[0].value, Maths::HVec3(0.16666667f, 0, 0.50666666f), 1.f);
	samples[0].tangentScale = 1.f;

	samples[1].tangentIn = Helpers::ControlPointToInTangent(samples[1].value, Maths::HVec3(0.33333331f, 0, 0.76666671f), 1.f);
	samples[1].tangentOut = Helpers::ControlPointToOutTangent(samples[1].value, Maths::HVec3(0.66666669f, 0, 1.6522919f), 1.f);
	samples[1].tangentScale = 1.f;

	samples[2].tangentIn = Helpers::ControlPointToInTangent(samples[2].value, Maths::HVec3(0.83333331f, 0, 0.20000000f), 1.f);
	samples[2].tangentOut = Maths::HVec3(0,0,0);
	samples[2].tangentScale = 1.f;

	C_HermiteSplineFlags flags;
	flags.calcKnotU = E_ProcessU_None;
	flags.calcTangents = false;
//	flags.arcSteps = nSamples*g_NewCurveReparamSteps;
	C_HermiteSpline<Maths::HVec3, C_CurveData<C_HermiteSplineKnot<Maths::HVec3> > > spline(flags);
	spline.SetData(&data);
	spline.Process();

	Maths::HMat43 transform;
	transform.SetIdentity();
	transform.AW(Maths::HVec3(-10,0,0));

	RenderHermiteSpline3D(
		DbUtil::DefaultDebugRenderSys()->Deferred(),
		spline,
		1.f,
		g_NewCurveUSteps,
		transform,
		Maths::RGBA(200,127,20), 
		Maths::RGBA(255,255,255),
		Maths::RGBA(255, 255, 128));

	
	C_Reparameterise reparam;
	reparam.BuildTable(&spline, 112);

	transform.AW(Maths::HVec3(-10,5,0));
	RenderReparamData(
		DbUtil::DefaultDebugRenderSys()->Deferred(),
		spline, 
		reparam,
		1.f, 
		transform, 
		Maths::RGBA(200,127,20),
		Maths::RGBA(255,255,255));
/*
	RenderArcStep(
		DbUtil::DefaultDebugRenderSys()->Deferred(), 
		spline,
		reparam,
		paramLength,
		g_NewCurveArcStepSize, 
		transform, 
		Maths::RGBA(190,0,190), 
		Maths::RGBA(0,0,0));
*/
	if (g_NewCurveTestPoint >= 0 && g_NewCurveTestPoint <= 1.f)
	{
		Maths::C_QuatTrans quatTrans;
		Actor::GetForwardFacingSplinePos(
			quatTrans,
			spline,
			g_NewCurveTestPoint,
			g_NewCurveTestPoint + 0.05f,
			false);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dMatrix(
			quatTrans.Matrix()*transform, 0.5f);
	}

	Maths::HVec3 pos = spline.Evaluate(g_NewCurveTestPoint);

	Maths::HVec3 nearestPt;
	F32 arcDistance = Actor::FindNearestPoint(
		spline,
		reparam,
		pos, 
		nearestPt);

	Maths::HVec3 tangent = spline.EvaluateTangent(reparam.GetU(arcDistance));
	tangent.Normalise();

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(
		(nearestPt-tangent*0.25)*transform, (nearestPt+tangent*0.25)*transform, Maths::RGBA(255,255,0));

	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(
		nearestPt*transform, 0.1f, Maths::RGBA(255,0,0));

	Maths::HVec3 closestPos;
	Maths::HVec3 tan;

	S32 idx = Actor::FollowerFindNearestKnotIndex(
		spline,
		pos,
		0);
	if (idx != -1)
	{
		F32 arcDist = Actor::FollowerFindNearestPointLinear(
			spline,
			pos,
			idx,
			closestPos, 
			tan);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(
			closestPos*transform, 0.2f, Maths::RGBA(128,60,255));
	}
	S32 idx2 = Actor::FollowerFindNearestReparamIndex(
		spline,
		reparam,
		pos,
		0);
	if (idx2 != -1)
	{
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(
			ReparamHelpers::GetPointValue(spline, reparam, idx2)*transform, 0.2f, Maths::RGBA(128,60,255));
		F32 arcDist2 = Actor::FollowerFindNearestPoint(
			spline,
			reparam,
			pos,
			idx2,
			closestPos, 
			tan);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dStar(
			closestPos*transform, 0.2f, Maths::RGBA(255,255,255));
	}
}

void HelloHermiteSpline1D(F32 paramLength)
{
	C_CurveData<C_HermiteSplineKnotU<F32> > data;
	Btl::Vector<C_HermiteSplineKnotU<F32> >& samples = data.GetKnots();

	U32 nSamples = 3;
	samples.Resize(nSamples);
	samples[0].SetU(0);		
	samples[1].SetU(0.5f);	
	samples[2].SetU(1);		
	samples[0].value = 0.34f;
	samples[1].value = 0.6f;
	samples[2].value = 0.2f;

	F32 period = 0.5f;
	samples[0].tangentIn = samples[0].tangentOut = Maths::Tan(Maths::DegToRad(45.f));
	samples[0].tangentScale = 1.f;
	samples[1].tangentIn = Maths::Tan(Maths::DegToRad(-45.f));
	samples[1].tangentOut = Maths::Tan(Maths::DegToRad(81.f));
	samples[1].tangentScale = 1.f;
	samples[2].tangentIn = samples[2].tangentOut = Maths::Tan(Maths::DegToRad(0.f));
	samples[2].tangentScale = 1.f;

	C_HermiteSplineFlags flags;
	flags.calcKnotU = E_ProcessU_None;
	flags.calcTangents = false;
	HermiteSpline1DAngular spline(flags);
	spline.SetData(&data);
	spline.Process();

	Maths::HMat43 transform;
	transform.SetIdentity();
	transform.AW(Maths::HVec3(-10,0,1));
	RenderHermiteSpline1D(
		DbUtil::DefaultDebugRenderSys()->Deferred(),
		spline, 
		1.f, 
		g_NewCurveUSteps,
		transform,
		Maths::RGBA(200,127,20), 
		Maths::RGBA(255,255,255),
		Maths::RGBA(255, 255, 128));
}

void HelloBSpline(F32 paramLength)
{
	//Draw a pentagon.
	C_CurveData<C_BSplineCv<Maths::HVec3> > data;
	Btl::Vector<C_BSplineCv<Maths::HVec3> >& samples = data.GetKnots();
	U32 nSamples = 6;

	samples.Resize(nSamples);
	samples[0].value = Maths::HVec3(1.235664, 0, -0.401492);
	samples[1].value = Maths::HVec3(0, 0, -1.299254);
	samples[2].value = Maths::HVec3(-1.235664, 0, -0.401492);
	samples[3].value = Maths::HVec3(-0.763682, 0, 1.051119);
	samples[4].value = Maths::HVec3(0.763682, 0, 1.051119);
	samples[5].value = Maths::HVec3( 1.235664, 0, -0.401492);

	C_BSpline<Maths::HVec3, C_CurveData<C_BSplineCv<Maths::HVec3> > > spline;
	spline.SetData(&data);
	spline.Process();

	Maths::HMat43 transform;
	transform.SetIdentity();
	transform.AW(Maths::HVec3(-6,0,0));
	RenderUStep(
		DbUtil::DefaultDebugRenderSys()->Deferred(),
		spline, 
		1.f,
		g_NewCurveUSteps,
		transform, 
		Maths::RGBA(255,0,255),
		Maths::RGBA(255,255,255));

	if (g_NewCurveTestPoint >= 0 && g_NewCurveTestPoint <= 1.f)
	{
		Maths::C_QuatTrans quatTrans;
		Actor::GetForwardFacingSplinePos(
			quatTrans,
			spline,
			g_NewCurveTestPoint,
			g_NewCurveTestPoint + 0.05f,
			false);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dMatrix(
			quatTrans.Matrix()*transform, 0.5f);
	}

	C_Reparameterise reparam;
	reparam.BuildTable(&spline, nSamples*g_NewCurveReparamRes);
	transform.AW(Maths::HVec3(-6,5,0));
	RenderArcStep(
		DbUtil::DefaultDebugRenderSys()->Deferred(), 
		spline,
		reparam,
		paramLength,
		g_NewCurveArcStepSize, 
		transform, 
		Maths::RGBA(255,0,255), 
		Maths::RGBA(255,255,255));
}

void HelloCardinalSpline(F32 paramLength)
{
	//Draw a pentagon.
	C_CurveData<C_CardinalSplineKnot<Maths::HVec3> > data;
	Btl::Vector<C_CardinalSplineKnot<Maths::HVec3> >& samples = data.GetKnots();
	U32 nSamples = 6;

	samples.Resize(nSamples);
	samples[0].value = Maths::HVec3(1.235664, 0, -0.401492);	samples[0].tangentScale = 1;
	samples[1].value = Maths::HVec3(0, 0, -1.299254);			samples[1].tangentScale = 1;
	samples[2].value = Maths::HVec3(-1.235664, 0, -0.401492);	samples[2].tangentScale = 1;
	samples[3].value = Maths::HVec3(-0.763682, 0, 1.051119);	samples[3].tangentScale = 1;
	samples[4].value = Maths::HVec3(0.763682, 0, 1.051119);		samples[4].tangentScale = 1;
	samples[5].value = Maths::HVec3( 1.235664, 0, -0.401492);	samples[5].tangentScale = 1;

	C_CardinalSplineFlags flags;
	flags.calcKnotU = E_ProcessU_None;
	C_CardinalSpline<Maths::HVec3, C_CurveData<C_CardinalSplineKnot<Maths::HVec3> > > spline(flags);
	spline.SetData(&data);
	spline.Process();

	Maths::HMat43 transform;
	transform.SetIdentity();
	transform.AW(Maths::HVec3(-3,0,0));
	RenderUStep(
		DbUtil::DefaultDebugRenderSys()->Deferred(),
		spline, 
		paramLength,
		g_NewCurveUSteps,
		transform,
		Maths::RGBA(0,0,255),
		Maths::RGBA(255,255,255));

	if (g_NewCurveTestPoint >= 0 && g_NewCurveTestPoint <= 1.f)
	{
		Maths::C_QuatTrans quatTrans;
		Actor::GetForwardFacingSplinePos(
			quatTrans,
			spline,
			g_NewCurveTestPoint,
			g_NewCurveTestPoint + 0.05f,
			false);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dMatrix(
			quatTrans.Matrix()*transform, 0.5f);
	}

	C_Reparameterise reparam;
	reparam.BuildTable(&spline, nSamples*g_NewCurveReparamRes);
	transform.AW(Maths::HVec3(-3,5,0));
	RenderArcStep(
		DbUtil::DefaultDebugRenderSys()->Deferred(), 
		spline,
		reparam,
		paramLength,
		g_NewCurveArcStepSize, 
		transform, 
		Maths::RGBA(0,0,255), 
		Maths::RGBA(255,255,255));
}

void HelloCatmullRomSpline(F32 paramLength)
{
	//Draw a pentagon.
	C_CurveData<C_CatmullRomSplineKnot<Maths::HVec3> > data;
	Btl::Vector<C_CatmullRomSplineKnot<Maths::HVec3> >& samples = data.GetKnots();
	U32 nSamples = 6;

	samples.Resize(nSamples);
	samples[0].value = Maths::HVec3(1.235664, 0, -0.401492);
	samples[1].value = Maths::HVec3(0, 0, -1.299254);
	samples[2].value = Maths::HVec3(-1.235664, 0, -0.401492);
	samples[3].value = Maths::HVec3(-0.763682, 0, 1.051119);
	samples[4].value = Maths::HVec3(0.763682, 0, 1.051119);
	samples[5].value = Maths::HVec3( 1.235664, 0, -0.401492);

	C_CatmullRomSplineFlags flags;
	flags.calcKnotU = E_ProcessU_None;
	C_CatmullRomSpline<Maths::HVec3, C_CurveData<C_CatmullRomSplineKnot<Maths::HVec3> > > spline(flags);
	spline.SetData(&data);
	spline.Process();
	
	Maths::HMat43 transform;
	transform.SetIdentity();
	transform.AW(Maths::HVec3(0,0,0));
	RenderUStep(
		DbUtil::DefaultDebugRenderSys()->Deferred(),
		spline, 
		paramLength,
		g_NewCurveUSteps,
		transform, 
		Maths::RGBA(128,50,200),
		Maths::RGBA(0,0,0));

	if (g_NewCurveTestPoint >= 0 && g_NewCurveTestPoint <= 1.f)
	{
		Maths::C_QuatTrans quatTrans;
		Actor::GetForwardFacingSplinePos(
			quatTrans,
			spline,
			g_NewCurveTestPoint,
			g_NewCurveTestPoint + 0.05f,
			false);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dMatrix(
			quatTrans.Matrix()*transform, 0.5f);
	}

	C_Reparameterise reparam;
	reparam.BuildTable(&spline, nSamples*g_NewCurveReparamRes);
	transform.AW(Maths::HVec3(0,5,0));
	RenderArcStep(
		DbUtil::DefaultDebugRenderSys()->Deferred(), 
		spline,
		reparam,
		paramLength,
		g_NewCurveArcStepSize, 
		transform, 
		Maths::RGBA(128,50,200), 
		Maths::RGBA(0,0,0));
}

void HelloBezierCurve(F32 paramLength)
{
	//Draw a pentagon.
	C_CurveData<C_BezierCurveKnot<Maths::HVec3> > data;
	Btl::Vector<C_BezierCurveKnot<Maths::HVec3> >& samples = data.GetKnots();
	U32 nSamples = 6;

	samples.Resize(nSamples);
	samples[0].value = Maths::HVec3(1.235664, 0, -0.401492);
	samples[1].value = Maths::HVec3(0, 0, -1.299254);
	samples[2].value = Maths::HVec3(-1.235664, 0, -0.401492);
	samples[3].value = Maths::HVec3(-0.763682, 0, 1.051119);
	samples[4].value = Maths::HVec3(0.763682, 0, 1.051119);	
	samples[5].value = Maths::HVec3( 1.235664, 0, -0.401492);

	C_BezierCurveFlags flags;
	flags.calcKnotU = E_ProcessU_None;
	flags.calcCtrlPoints = true;
	C_BezierCurve<Maths::HVec3, C_CurveData<C_BezierCurveKnot<Maths::HVec3> > > spline(flags);
	spline.SetData(&data);
	spline.Process();

	Maths::HMat43 transform;
	transform.SetIdentity();
	transform.AW(Maths::HVec3(3,0,0));

	RenderBezierCurve(
		DbUtil::DefaultDebugRenderSys()->Deferred(),
		spline, 
		paramLength,
		g_NewCurveUSteps,
		transform,
		Maths::RGBA(200,255,20),
		Maths::RGBA(0,0,0),
		Maths::RGBA(255, 255, 128));

	if (g_NewCurveTestPoint >= 0 && g_NewCurveTestPoint <= 1.f)
	{
		Maths::C_QuatTrans quatTrans;
		Actor::GetForwardFacingSplinePos(
			quatTrans,
			spline,
			g_NewCurveTestPoint,
			g_NewCurveTestPoint + 0.05f,
			false);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dMatrix(
			quatTrans.Matrix()*transform, 0.5f);
	}

	transform.AW(Maths::HVec3(3,5,0));

	C_Reparameterise reparam;
	reparam.BuildTable(&spline, nSamples*g_NewCurveReparamRes);

	RenderArcStep(
		DbUtil::DefaultDebugRenderSys()->Deferred(), 
		spline,
		reparam,
		paramLength,
		g_NewCurveArcStepSize,
		transform, 
		Maths::RGBA(255,255,0), 
		Maths::RGBA(0,0,0));
}

void HelloMayaHermiteSpline(F32 paramLength)
{
	C_CurveData<C_MayaHermiteSplineKnot<Maths::HVec3> > data;
	Btl::Vector<C_MayaHermiteSplineKnot<Maths::HVec3> >& samples = data.GetKnots();

	U32 nSamples = 5;
	samples.Resize(nSamples);
	samples[0].value = Maths::HVec3(0, 0, -2);
	samples[1].value = Maths::HVec3(2, 0, -1);
	samples[2].value = Maths::HVec3(0, 0, 0);
	samples[3].value = Maths::HVec3(1, 0, 1);
	samples[4].value = Maths::HVec3(0, 0, 2);

	samples[0].tangentScale = 1.f;
	samples[1].tangentScale = 1.f;
	samples[2].tangentScale = 1.f;
	samples[3].tangentScale = 1.f;
	samples[4].tangentScale = 1.f;

	C_MayaHermiteSplineFlags flags;
	flags.calcKnotU = E_ProcessU_None;
	flags.calcTangents = true;
	C_MayaHermiteSpline<Maths::HVec3, C_CurveData<C_MayaHermiteSplineKnot<Maths::HVec3> > > spline(flags);
	spline.SetData(&data);
	spline.Process();

	Maths::RGBA segA = Maths::RGBA(0,127,200);
	Maths::RGBA segB = Maths::RGBA(100,200,150);

	Maths::HMat43 transform;
	transform.SetIdentity();
	transform.AW(Maths::HVec3(6,0,0));

//	spline.SetPreInfinity(E_InfinityType_Oscillate);
//	spline.SetPostInfinity(E_InfinityType_Oscillate);
	spline.SetPreInfinity(E_InfinityType_CycleWithOffset);
	spline.SetPostInfinity(E_InfinityType_CycleWithOffset);
//	spline.SetPreInfinity(E_InfinityType_Linear);
//	spline.SetPostInfinity(E_InfinityType_Linear);

	U32 nSteps = g_NewCurveUSteps;
	F32 tStep = paramLength/nSteps;

	F32 tStart = -0.99f;
	Maths::HVec3 prevPosition = spline.Evaluate(tStart);
	F32 t = tStart + tStep;
	for(U32 i = 1; i <= nSteps; ++i, t +=tStep*3)
	{
		Maths::HVec3 nextPosition = spline.Evaluate(t);

		Maths::RGBA colour = i&1 ? segA : segB;
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dLine(
			prevPosition*transform, 
			nextPosition*transform, 
			colour);
		prevPosition = nextPosition;
	}

	Maths::C_QuatTrans quatTrans;
	Actor::GetForwardFacingSplinePos(
		quatTrans,
		spline,
		g_NewCurveTestPoint,
		g_NewCurveTestPoint + 0.05f,
		false);
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dMatrix(
		quatTrans.Matrix()*transform, 0.5f);

	C_Reparameterise reparam;
	reparam.BuildTable(&spline, nSamples*g_NewCurveReparamRes);
	transform.AW(Maths::HVec3(6,5,0));
	RenderArcStep(
		DbUtil::DefaultDebugRenderSys()->Deferred(), 
		spline,
		reparam,
		paramLength,
		g_NewCurveArcStepSize, 
		transform, 
		segA, 
		segB);
}

void HelloNurbsSpline(F32 paramLength)
{
	//Draw a NURBS circle. Curve is cubic, non rational (all weights are 1) and uniform (even spacing in parameter space).
	C_NurbSplineData<Maths::HVec3> data;
	Btl::Vector<C_NurbsCv<Maths::HVec3> >& cvs = data.GetCvs();
	U32 nCvs = 8;
	U32 degree = 3;
	U32 order = degree + 1;

	cvs.Resize(nCvs);
	cvs[0].value = Maths::HVec3(1.235664, 0, -0.401492);	cvs[0].weight = 1.f;
	cvs[1].value = Maths::HVec3(0, 0, -1.299254);			cvs[1].weight = 1.f;
	cvs[2].value = Maths::HVec3(-1.235664, 0, -0.401492);	cvs[2].weight = 1.f;
	cvs[3].value = Maths::HVec3(-0.763682, 0, 1.051119);	cvs[3].weight = 1.f;
	cvs[4].value = Maths::HVec3(0.763682, 0, 1.051119);		cvs[4].weight = 1.f;
	cvs[5].value = Maths::HVec3( 1.235664, 0, -0.401492);	cvs[5].weight = 1.f;
	cvs[6].value = Maths::HVec3(0, 0, -1.299254);			cvs[6].weight = 1.f;
	cvs[7].value = Maths::HVec3(-1.235664, 0, -0.401492);	cvs[7].weight = 1.f;

	Btl::Vector<F32>& knots = data.GetKnots();
	knots.Resize(nCvs + order);
	knots[0] = -2;
	knots[1] = -1;
	knots[2] = 0;
	knots[3] = 1;
	knots[4] = 2;
	knots[5] = 3;
	knots[6] = 4;
	knots[7] = 5;
	knots[8] = 6;
	knots[9] = 7;
	knots[10] = 8;
	knots[11] = 9;

	C_NurbSpline<Maths::HVec3, C_NurbSplineData<Maths::HVec3> > spline(degree);
	spline.SetData(&data);
	spline.Process();

	Maths::HMat43 transform;
	transform.SetIdentity();
	transform.AW(Maths::HVec3(9,0,0));
	RenderUStep(
		DbUtil::DefaultDebugRenderSys()->Deferred(), 
		spline,
		paramLength,
		g_NewCurveUSteps,
		transform,
		Maths::RGBA(255,255,255),
		Maths::RGBA(0,0,0));

	Maths::C_QuatTrans quatTrans;
	Actor::GetForwardFacingSplinePos(
		quatTrans,
		spline,
		g_NewCurveTestPoint,
		g_NewCurveTestPoint + 0.05f,
		false);
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dMatrix(
		quatTrans.Matrix()*transform, 0.5f);

	C_Reparameterise reparam;
	reparam.BuildTable(&spline, nCvs*g_NewCurveReparamRes);
	transform.AW(Maths::HVec3(9,5,0));
	RenderArcStep(
		DbUtil::DefaultDebugRenderSys()->Deferred(), 
		spline,
		reparam,
		paramLength,
		g_NewCurveArcStepSize, 
		transform, 
		Maths::RGBA(255,255,255),
		Maths::RGBA(0,0,0));
}

void HelloPolyline(F32 paramLength)
{
	//Draw a pentagon.
	C_CurveData<C_Knot<Maths::HVec3> > data;
	Btl::Vector<C_Knot<Maths::HVec3> >& samples = data.GetKnots();
	U32 nSamples = 6;

	samples.Resize(nSamples);
	samples[0].value = Maths::HVec3(1.235664, 0, -0.401492);
	samples[1].value = Maths::HVec3(0, 0, -1.299254);
	samples[2].value = Maths::HVec3(-1.235664, 0, -0.401492);
	samples[3].value = Maths::HVec3(-0.763682, 0, 1.051119);
	samples[4].value = Maths::HVec3(0.763682, 0, 1.051119);
	samples[5].value = Maths::HVec3( 1.235664, 0, -0.401492);

	C_PolyLineFlags flags;
	flags.calcKnotU = E_ProcessU_None;
	C_PolyLine<Maths::HVec3, C_CurveData<C_Knot<Maths::HVec3> > > polyLine(flags);
	polyLine.SetData(&data);
	if (!polyLine.Process())
	{
		assert(!"Failed to process!");
		return;
	}

	Maths::HMat43 transform;
	transform.SetIdentity();
	transform.AW(Maths::HVec3(12,0,0));
	RenderUStep(
		DbUtil::DefaultDebugRenderSys()->Deferred(), 
		polyLine,
		paramLength, 
		40,
		transform, 
		Maths::RGBA(255,0,0), 
		Maths::RGBA(0,0,0));

	Maths::C_QuatTrans quatTrans;
	Actor::GetForwardFacingSplinePos(
		quatTrans,
		polyLine,
		g_NewCurveTestPoint,
		g_NewCurveTestPoint + 0.05f,
		false);
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dMatrix(
		quatTrans.Matrix()*transform, 0.5f);
}

void HelloWorld(F32 paramLength)
{
	HelloHermiteSpline(paramLength);
	HelloHermiteSpline1D(paramLength);
	HelloBSpline(paramLength);
	HelloCardinalSpline(paramLength);
	HelloCatmullRomSpline(paramLength);
	HelloBezierCurve(paramLength);
	HelloMayaHermiteSpline(paramLength);
	HelloNurbsSpline(paramLength);
	HelloPolyline(paramLength);
}

}
