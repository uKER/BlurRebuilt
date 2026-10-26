#include "stdafx.h"
#include <spline/cspline.h>


#ifdef _MSC_VER
#ifdef DEBUG_AI
#ifdef CFG_ALLOW_OPTIMIZE_OFF
#pragma optimize("", off)
#endif
#endif
#endif

namespace airacingtech
{
	CSplineAI::CSplineAI()
	{
	}

	CSplineAI::~CSplineAI()
	{

	}

	void CSplineAI::RegisterComponentType()
	{
		GlobalsComponent::GetObjectManager().RegisterInterfaceWithComponent(IID_SPLINE_AI, CID_SPLINE_AI);
		GlobalsComponent::GetObjectManager().RegisterComponentType(CID_SPLINE_AI, CSplineAI::CreateMe, CSplineAI::DestroyMe, CHash("SplineAI"));
	}

	IComponent *CSplineAI::CreateMe()
	{
		return new CSplineAI;
	}

	bool CSplineAI::DestroyMe(IComponent *pComponent)
	{
		delete pComponent;

		return true;
	}

	// Virtual IComponent methods
	bool CSplineAI::Init(CObjectId oid, CParameterNode &paramNode)
	{
		SplineInitialisationParams *pParams = static_cast<SplineInitialisationParams*>(paramNode.GetData());
		if (pParams)
		{
			if (pParams->m_resample == true)
			{
				// resample to 1 metre intervals........
				B_Spline_ApproxNonLooped(pParams->m_controlPoints, m_SamplePoints, 1.0f, 0.0005f);
				SetLength((int)m_SamplePoints.Size());
				SetInvLength((float)(1.0f / m_Length));
			}
			else
			{
				// just copy the control points as sample points as they are roughyl
			}
		}


		/*
		NavigationSplineData *pParams = static_cast<NavigationSplineData *>( paramNode.GetData() );

		if (pParams->m_makeBSpline == true)
		{
			Ut::UtVector<Ut::DynamicVector<Maths::HVec3> > spoints;
			airacingtech::B_Spline_Approx(pParams->m_SamplePoints, spoints, 1.0f, 0.0025f); // 1.0f
			SetSamplePoints(spoints);
			SetOpenClosed(pParams->m_Open);
			// todo Tangent normal for splines created this way
		}
		else
		{
			SetSamplePoints(pParams->m_SamplePoints);
			
			//if (pParams->m_Open == false)
			//{	
			//	JoinLooped();
			//}

			SetOpenClosed(pParams->m_Open);
			SetTangentNormal(&pParams->m_TangentNormal);
		}
*/
		return true;
	}

	void CSplineAI::Deinit(void)
	{
		m_SamplePoints.Clear();
		m_TangentNormal.Clear();
		m_Length = -1;
		m_InvLength = 0.0f;
		m_Open = false;
	}

	EMessageResult CSplineAI::HandleMessage(const CMessage &msg)
	{
		EMessageResult msgRet = MR_IGNORED;

		return msgRet;
	}


	EComponentTypeId CSplineAI::GetComponentTypeId(void)
	{
		return CID_SPLINE_AI;
	}

	
	
}

#ifdef _MSC_VER
#ifdef DEBUG_AI
#pragma optimize("", on)
#endif
#endif


