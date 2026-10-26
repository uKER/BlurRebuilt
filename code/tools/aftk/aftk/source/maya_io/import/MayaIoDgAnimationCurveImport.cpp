#include "Stdafx.h"

using namespace MayaIo;

Aftk::C_Status C_DgAnimationCurveImport::Import(
	C_Importer* pImporter,
	Aftk::C_Scene* pAftkScene,
	MFnDependencyNode& fnDependencyNode,
	Aftk::C_Node* pAftkNode)
{
	Aftk::C_AnimationCurveNode* pAftkAnimationCurve
		= (Aftk::C_AnimationCurveNode*)pAftkNode;

	Aftk::C_Status status = C_DgNodeImport::Import(
		pImporter,
		pAftkScene,
		fnDependencyNode,
		pAftkAnimationCurve);
	if (!status) return status;

	MStatus mStatus;

	MFnAnimCurve fnAnimCurve(fnDependencyNode.object(), &mStatus);
	MCHECK_ERROR_RETURN_1;

	MTime oneSecond( 1, MTime::kSeconds );
	F64 frameRate = oneSecond.as(MTime::uiUnit());

	F32 lastTime = 0;
	for (U32 iKey = 0; iKey < fnAnimCurve.numKeys(); iKey++ ) 
	{
		F32 sampleX;
		if (fnAnimCurve.isUnitlessInput())
		{
			sampleX = (F32)fnAnimCurve.unitlessInput(iKey, &mStatus);
			AFTK_CHECK_ERROR_RETURN;
		}
		else
		{
			F64 timeSeconds = fnAnimCurve.time(iKey).as(MTime::kSeconds);
            sampleX = (F32)(timeSeconds * frameRate);
		}
		
		F32 value = (F32)fnAnimCurve.value(iKey);
		Aftk::C_HermiteKey aftkKey;
		aftkKey.SetX(sampleX);
		aftkKey.SetValue(value);
		
		MAngle inTangentAngle;
		double inWeight;
		fnAnimCurve.getTangent(iKey, inTangentAngle, inWeight, true);

		MAngle outTangentAngle;
		double outWeight;
		fnAnimCurve.getTangent(iKey, outTangentAngle, outWeight, false);

		/*
		The x and y values of the anim curve tangents are stored in the internal
		units of seconds, centimeters and radians. They need to be converted to the
		units of your anim curve.

		The example I have seen states that if the x axis is in film units of 24
		frames/sec, then one second of time would contain 24 film units. Thus the x
		value returned from the tangent should be multiplied by 24 to give the
		correct value.

		If the yaxis units of your anim curve is millimeters then you need to
		mulitiply the returned y value by 10 (because there are 10 mm in a cm)

		If the anim curve is unitless then you do not need to convert.
		*/

		MFnAnimCurve::AnimCurveType curveType = fnAnimCurve.animCurveType();

		if (curveType == MFnAnimCurve::kAnimCurveTA)
		{
			MTime convert(1.0, MTime::kSeconds);
			F32 inTangent, outTangent;
			{
				Aftk::Vector2F32 inTan;
				fnAnimCurve.getTangent(iKey, inTan[0], inTan[1], true);

				inTan[0] *= (F32)(convert.as(MTime::kSeconds)*frameRate);
				inTangent = atan2(inTan[1], inTan[0]);
			}
			{
				Aftk::Vector2F32 outTan;
				fnAnimCurve.getTangent(iKey, outTan[0], outTan[1], false);

				outTan[0] *= (F32)(convert.as(MTime::kSeconds)*frameRate);
				outTangent = atan2(outTan[1], outTan[0]);
			}
			//Compensate for angles being stored in degrees by maya.
			aftkKey.SetInTangentAngle(inTangent);
			aftkKey.SetOutTangentAngle(outTangent);
		}
		else if (curveType == MFnAnimCurve::kAnimCurveUA)
		{
			F32 inTangent, outTangent;
			{
				Aftk::Vector2F32 inTan;
				fnAnimCurve.getTangent(iKey, inTan[0], inTan[1], true);

				inTangent = atan2(inTan[1], inTan[0]);
			}
			{
				Aftk::Vector2F32 outTan;
				fnAnimCurve.getTangent(iKey, outTan[0], outTan[1], false);
				outTangent = atan2(outTan[1], outTan[0]);
			}
			//Compensate for angles being stored in degrees by maya.
			aftkKey.SetInTangentAngle(inTangent);
			aftkKey.SetOutTangentAngle(outTangent);
		}
		else
		{
			aftkKey.SetInTangentAngle((F32)inTangentAngle.asRadians());
			aftkKey.SetOutTangentAngle((F32)outTangentAngle.asRadians());
		}
		
		aftkKey.SetInTangentType(GetAftkTangentType(fnAnimCurve.inTangentType(iKey)));
		aftkKey.SetOutTangentType(GetAftkTangentType(fnAnimCurve.outTangentType(iKey)));
		pAftkAnimationCurve->AddKey(aftkKey);
	}
		
	pAftkAnimationCurve->SetPreInfinity(
		GetAftkInfinityType(fnAnimCurve.preInfinityType()));
	pAftkAnimationCurve->SetPostInfinity(
		GetAftkInfinityType(fnAnimCurve.postInfinityType()));

	//Set if this curve is driven by the scene time.
	pAftkAnimationCurve->SetIsTimeInput(fnAnimCurve.isTimeInput());

	#ifdef _DEBUG
	AFTK_DO_BEGIN << "Curve start/end time " << pAftkAnimationCurve->GetStartX() << " " << pAftkAnimationCurve->GetEndX() << "\n" AFTK_DO_DEBUG_END;
	if (pAftkAnimationCurve->IsTimeInput())
		AFTK_DO_BEGIN << "Curve accepts time as its input.\n"  AFTK_DO_DEBUG_END;
	#endif

	//Examine the animation curve's input connection, exporting chained nodes.
	{
		MPlug input = fnAnimCurve.findPlug("input", &mStatus);
		MCHECK_ERROR_RETURN_1;

		Aftk::I_Object* pChainedObject = 
			C_ConnectedDgImport::ImportDirectlyConnectedPlug(
				pImporter,
				input,
				status);
			AFTK_CHECK_ERROR_RETURN;

		//Add animation curve input connection.
		pAftkAnimationCurve->GetInputPlug().SetSourceConnectionPtr(pChainedObject);
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::E_CurveTangentType C_DgAnimationCurveImport::GetAftkTangentType(MFnAnimCurve::TangentType type)
{
	switch(type)
	{
	case MFnAnimCurve::kTangentStep:
		return Aftk::E_CurveTangentType_Stepped;
 	case MFnAnimCurve::kTangentClamped:
		return Aftk::E_CurveTangentType_Clamped;
 	case MFnAnimCurve::kTangentStepNext:
		return Aftk::E_CurveTangentType_SteppedNext;
	default:
		return Aftk::E_CurveTangentType_Unknown;
 	};
}

Aftk::E_CurveInfinityType C_DgAnimationCurveImport::GetAftkInfinityType(MFnAnimCurve::InfinityType type)
{
	switch(type)
	{
	case MFnAnimCurve::kLinear:
		return Aftk::E_CurveInfinityType_Linear;
	case MFnAnimCurve::kCycle:
		return Aftk::E_CurveInfinityType_Cycle;
	case MFnAnimCurve::kCycleRelative:
		return Aftk::E_CurveInfinityType_CycleWithOffset;
	case MFnAnimCurve::kOscillate:
		return Aftk::E_CurveInfinityType_Oscillate;
	case MFnAnimCurve::kConstant:
	default:
		return Aftk::E_CurveInfinityType_Constant;
	}
}
